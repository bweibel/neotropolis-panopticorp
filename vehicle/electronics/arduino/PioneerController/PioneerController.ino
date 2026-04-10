// =============================================================================
// PioneerController — Arduino Uno R4 WiFi
// Role: Reads physical inputs, controls Pioneer head unit via X9C104 digital
//       pot, and broadcasts serial events to ESP32 Interior Hub.
//
// See: vehicle/electronics/baja-lighting-spec.md
// =============================================================================

#include <SoftwareSerial.h>
#include "X9C.h"
#include "pt.h"

// =============================================================================
// Pin constants
// =============================================================================

const int ENC_CLK    = 2;   // Rotary encoder CLK
const int ENC_DT     = 3;   // Rotary encoder DT
const int ENC_SW     = 4;   // Rotary encoder button (mute)
const int BTN_1      = 5;   // Skip forward (SCAN)
const int BTN_2      = 6;   // Skip back (SWEEP)
const int BTN_3      = 7;   // Scene cycle (PURGE)
const int BTN_4      = 8;   // Reserved (UPLINK)
const int TOGGLE_LED = 9;   // T2: LED master power state

// SoftwareSerial TX/RX pins (one-way TX to ESP32 hub; RX unused)
const int SERIAL_TX  = 10;
const int SERIAL_RX  = A0;  // unused (one-way TX only) but required by SoftwareSerial constructor

// Digital pot (X9C104) pins
const int PIN_POT_CS  = 13;
const int PIN_POT_UD  = 12;
const int PIN_POT_INC = 11;

// =============================================================================
// Event string constants
// =============================================================================

const char* EVT_VOL_UP    = "VOL_UP\n";
const char* EVT_VOL_DOWN  = "VOL_DOWN\n";
const char* EVT_MUTE      = "MUTE\n";
const char* EVT_SKIP_FWD  = "SKIP_FWD\n";
const char* EVT_SKIP_BACK = "SKIP_BACK\n";
const char* EVT_SCENE_NEXT = "SCENE_NEXT\n";

// =============================================================================
// Serial link
// =============================================================================

SoftwareSerial espSerial(SERIAL_RX, SERIAL_TX);

// =============================================================================
// State
// =============================================================================

bool ledMasterOn = false;

// =============================================================================
// Globals — encoder
// =============================================================================

// for debugging do a find/replace on Serial with (slash)(slash)Serial

// Encoder vars
int                 EncoderValue                  = 0;
int                 PreviousEncoderVal            = 0;
int                 Delta                         = 0;
int                 PreviousPush                  = 0;
static int          oldA                          = HIGH;
static int          oldB                          = HIGH;

// time vars
unsigned long       CurrentMills                  = 0;
unsigned long       PreviousMills                 = 0;
static int          DoubleClickTime               = 350;         // how much time between single click and if another "double" click
static int          DeBounceDelay                 = 10;          // this is hardware/software loop centric, for my setup and loop() 1 seems to be working well
static int          MinSliceDelay                 = 1;
static int          MoreAggressiveDebounce        = 200;

// Threading times                                              //  libraries scheduler seem to need different CPUs, and protothread seemed to involved, so just did a simple wait schedule with anti-stravation
static int          WaitForUnitToComplete         = 41;         // so far it looks like the Pioneer might need 40msec to respond to the event
static int          WaitForDisplayTime            = 850;        // was 650        // this should be the minimum time to display the screen
static int          WaitTimeForBetweenScreens     = 4100;       // this should be the minimum time for the screen to remove after no other commands have been sent

// hold down button for Mute method
static bool         HoldDownForMute               = true;      // this is just a toggle to see if you are using this method of muting
static int          HowLongToHoldForMute          = 500;       // this probably should be more than the doubleclicktime (cause less confusion)

// ProtoThreads
static struct pt pt1, pt2, pt3;

int lastToggleState = HIGH;  // initialized in setup() from actual pin state — avoids spurious change on first loop

// ProtoThread Queue
#define               QUEUEMAXSIZE  200
int                   QueueCommands[QUEUEMAXSIZE];
int                   QueueIndex=0;
int                   QueueLastProc=0;
static int            LoopOfThread                  = 0;
static int            CurLimit                      = 0;

// 20200714 -- you must now place the defines for anything that causes the Pioneer VolumeScreen to come up first and contiguous so that SCREENRANGE is < all of them
#define VOLUMEUP      1
#define VOLUMEDOWN    2
#define MUTE          3         // mute can be implemented as the button MUTE or PLAY/PAUSE which would allow Navigation to continue, I usually opt for that method
#define SCREENRANGE   4

#define TRACKFF       4
#define TRACKPV       5
#define TRIPLECLICK   6       // unsure what this does, mostly testing right now

// Globals — misc
#define             NumOfStepsNeeded               1    // given the travel, you might want increase the number of steps needed for a volume move (my encoder seems too aggressively moving)

// =============================================================================
// X9C pot
// =============================================================================

// in the case of the X9C pot, its a percentage and the 104 is 100K with 100 steps so each step is 1000 Ohm or 1K so you just set the percentage directly

#define REST_VOLUMEUP                              15
#define REST_VOLUMEDOWN                            23
#define REST_TRACKFF                               7
#define REST_MUTE                                  2
#define REST_TRACKPV                               10
#define REST_TRIPLECLICK                           0

// create a global POT object
X9C pot;

// =============================================================================
// setup / loop
// =============================================================================

void setup()
{
  espSerial.begin(9600);

  PT_INIT(&pt1);
  PT_INIT(&pt2);
  PT_INIT(&pt3);

  // setup encoder
  pinMode(ENC_CLK,    INPUT_PULLUP);
  pinMode(ENC_DT,     INPUT_PULLUP);
  pinMode(ENC_SW,     INPUT_PULLUP);
  digitalWrite(ENC_SW, HIGH);

  pinMode(BTN_1,      INPUT_PULLUP);
  pinMode(BTN_2,      INPUT_PULLUP);
  pinMode(BTN_3,      INPUT_PULLUP);
  pinMode(BTN_4,      INPUT_PULLUP);
  pinMode(TOGGLE_LED, INPUT_PULLUP);

  ledMasterOn = (digitalRead(TOGGLE_LED) == LOW);
  lastToggleState = digitalRead(TOGGLE_LED);

  // setup POT
  pot.begin(PIN_POT_CS, PIN_POT_INC, PIN_POT_UD);
  delay(1);
  pot.setPotMax(true);
  delay(WaitForUnitToComplete);

  // clear the queue, I believe this can be removed since the units memory starts zero'ed, (I remember reading that somewhere)
  ClearQueue();

  // Setup output
  Serial.begin(115200);
  Serial.println("Started");

  // TODO: initialize LCD
}

void loop()
{
  protothread1(&pt1);
  protothread2(&pt2);
  protothread3(&pt3);
}

// =============================================================================
// Protothreads
// =============================================================================

static int protothread1(struct pt *pt)
{
  static unsigned long timestamp            = 0;
  static int           change               = 0;

  static unsigned long LastTimeDirection    = 0;
  static int           LastDirection        = 0;

  static unsigned long PrevLongHoldTime     = 0;
  static unsigned long StillHoldingDown     = 0;
  PT_BEGIN(pt);
  while(1)
  {
      // get encode info
      change              = getEncoderTurn();
      PreviousEncoderVal  = EncoderValue;
      EncoderValue        = EncoderValue+change;

      // see if they clicked, and if they did, and the time has expired, then track forward
      if (PreviousMills != 0 && millis()-PreviousMills>DoubleClickTime)
      {
          // you have to see if you are holding down just in case because you dont want to track AND mute
          if (HoldDownForMute && PrevLongHoldTime)
          {
            Serial.println("You are still holding down, dont track forward until you let go");
          }
          else
         {
            PreviousMills =0;
            PulseTrackForward();
         }
      }


      // reset if you push the button, that will probably change to track forward, and 2 is track back
      if(digitalRead(ENC_SW) == LOW)
      {
        // this is the handler for Muting now
        if (HoldDownForMute)
       {
          if (PrevLongHoldTime && millis()-PrevLongHoldTime > HowLongToHoldForMute && !StillHoldingDown)
          {
              StillHoldingDown = millis();
              Serial.println("You have held down long enough");
              ClearQueue();
              PulseMute();
              //timestamp = millis(); PT_WAIT_UNTIL(pt, millis() - timestamp > WaitForUnitToComplete);                        // allow other thread some time
              Serial.println("DONE FORCE MUTE");
              PreviousMills=0; // this is here to make sure that the next command isnt a TrackForward
          }
          if (PrevLongHoldTime == 0)
            PrevLongHoldTime = millis();
       }

        if (PreviousPush==0)
        {
           CurrentMills = millis();
           if (PreviousMills==0)
           {
             PreviousMills = CurrentMills;
           }
           else if (CurrentMills-PreviousMills<DoubleClickTime)
           {
                PreviousMills =0;
                PulseTrackBack();
           }
           PreviousPush = 1;
        }
        timestamp = millis(); PT_WAIT_UNTIL(pt, millis() - timestamp > DeBounceDelay);
      }
      else
      {
          PreviousPush = 0;
          if (HoldDownForMute)
          {
              PrevLongHoldTime = 0;
              StillHoldingDown = 0;
          }
      }


      if (EncoderValue !=  PreviousEncoderVal)
      {
         Delta = Delta + EncoderValue - PreviousEncoderVal;
         if (Delta == NumOfStepsNeeded || Delta == -NumOfStepsNeeded)
         {
           if (Delta >0)
           {
                Delta=0;
                if (LastDirection==VOLUMEDOWN && millis() - LastTimeDirection < MoreAggressiveDebounce)
                {
                  Serial.println((String)"This UP is being ignored, TimeDiff~=" + (millis() - LastTimeDirection));
                }
                else
                {
                  PulseVolumeUp();
                  LastDirection=VOLUMEUP;
                }
                LastTimeDirection=millis();
           }
           else
           {
                Delta=0;

                if (LastDirection==VOLUMEUP && millis() - LastTimeDirection < MoreAggressiveDebounce)
                {
                  Serial.println((String)"This DOWN is being ignored, timediff~= " + (millis() - LastTimeDirection));
                }
                else
                {
                  PulseVolumeDown();
                  LastDirection=VOLUMEDOWN;
                  LastTimeDirection=millis();
                }
             }
         }
      }
      timestamp = millis(); PT_WAIT_UNTIL(pt, millis() - timestamp > MinSliceDelay);                        // allow other thread some time
  }
  PT_END(pt);
}

static int protothread2(struct pt *pt)
{
  static unsigned long  timestamp                      = 0;
  static uint16_t       Command                        = 0;
  static unsigned long  PreviousCommandTimeStamp       = 0;
  static bool           WaitForDisplay                 =false;
  static unsigned long  TimeWhenInQueue                = 0;
  static bool           InsideAQueueProcess            =false;
  static int            TempCommand                    =0;


  PT_BEGIN(pt);
  while(1)
  {
     Command=0;
      CurLimit=QueueIndex;
      if (CurLimit<QueueLastProc)
        CurLimit=CurLimit+QUEUEMAXSIZE;

      if (QueueLastProc != CurLimit)
      {
        Serial.println((String)"QueueIndex="+QueueIndex+" QueueLastProc="+QueueLastProc+ " CurLimit="+CurLimit+" LoopOfThread="+LoopOfThread);
        for (LoopOfThread = QueueLastProc; LoopOfThread < CurLimit; LoopOfThread++)
        {
            Command=0;
            switch(QueueCommands[LoopOfThread%QUEUEMAXSIZE])
            {
              case VOLUMEUP:
                Command = REST_VOLUMEUP;
                Serial.print("UP ");
                break;
              case VOLUMEDOWN:
                Command = REST_VOLUMEDOWN;
                Serial.print("DOWN ");
                break;
              case TRACKFF:
                Command = REST_TRACKFF;
                Serial.print("FF ");
                break;
              case TRACKPV:
                Command = REST_TRACKPV;
                Serial.print("PV ");
                break;
              case MUTE:
                Command = REST_MUTE;
                Serial.print("MUTE ");
                break;
//              case TRIPLECLICK:
//                Command = REST_TRIPLECLICK;
//                Serial.print("TRIPLECLICK ");
//                break;
              default:
                Serial.println((String)"Dont think I should hit these QueueIndex="+QueueIndex+" QueueLastProc="+QueueLastProc+ " CurLimit="+CurLimit);
                Command=0;
                timestamp = millis();PT_WAIT_UNTIL(pt, millis() - timestamp > DeBounceDelay);
              break;
            }
            TempCommand = QueueCommands[LoopOfThread%QUEUEMAXSIZE];
            QueueCommands[LoopOfThread%QUEUEMAXSIZE]=0;

            if (Command != 0)
            {
                timestamp = millis();
                if (timestamp-PreviousCommandTimeStamp > WaitTimeForBetweenScreens && !InsideAQueueProcess && TempCommand < SCREENRANGE)    // SCREENRANGE must be +1 then ALL display impacting cases
                {
                  PreviousCommandTimeStamp = timestamp;
                  WaitForDisplay=true;
                }

                InsideAQueueProcess=true;
                TimeWhenInQueue = millis();


                pot.setPot(Command,false);
                timestamp = millis();PT_WAIT_UNTIL(pt, millis() - timestamp > WaitForUnitToComplete);                       // allow stereo time to handle the input
                Serial.println("CommandDone");

                pot.setPotMax(true);
                timestamp = millis();PT_WAIT_UNTIL(pt, millis() - timestamp > WaitForUnitToComplete);                       // allow stereo time to handle the input

                if (WaitForDisplay)
                {
                   Serial.println("Waiting for Screen");
                   WaitForDisplay = false;
                   timestamp = millis();PT_WAIT_UNTIL(pt, millis() - timestamp > WaitForDisplayTime);                // allow stereo time to handle the input
                   Serial.println("Screen should be up, do any other commands");
                }
            }
            Command=0;
        } // when the for-next loop is done (all the commands on the queue)
        QueueLastProc=CurLimit%QUEUEMAXSIZE;
      }
      else // if you are here there was no messages in the queue
      {
        timestamp = millis();
        if (timestamp-TimeWhenInQueue > WaitTimeForBetweenScreens)
        {
           if (InsideAQueueProcess)
              Serial.println("The screen is no longer on ");
           InsideAQueueProcess=false;
        }
      }
      timestamp = millis(); PT_WAIT_UNTIL(pt, millis() - timestamp > MinSliceDelay);                                // allow other thread some time
  }
  PT_END(pt);
}

static int protothread3(struct pt *pt)
{
  static unsigned long timestamp = 0;
  static int lastBtn1 = HIGH;
  static int lastBtn2 = HIGH;
  static int lastBtn3 = HIGH;
  // lastToggleState is a global set in setup() from the actual pin — avoids spurious change on first loop
  // (a static local can't be initialized from a runtime value)

  PT_BEGIN(pt);
  while (1)
  {
    int btn1 = digitalRead(BTN_1);
    int btn2 = digitalRead(BTN_2);
    int btn3 = digitalRead(BTN_3);
    int tog  = digitalRead(TOGGLE_LED);
    int& lastToggle = lastToggleState;  // alias to global set in setup()

    // BTN_1: Skip forward
    if (btn1 == LOW && lastBtn1 == HIGH) {
      PulseTrackForward();  // queues Pioneer command + emits EVT_SKIP_FWD
    }
    lastBtn1 = btn1;

    // BTN_2: Skip back
    if (btn2 == LOW && lastBtn2 == HIGH) {
      PulseTrackBack();     // queues Pioneer command + emits EVT_SKIP_BACK
    }
    lastBtn2 = btn2;

    // BTN_3: Scene cycle — serial only, no Pioneer command, suppressed when LED master off
    if (btn3 == LOW && lastBtn3 == HIGH) {
      if (ledMasterOn) {
        espSerial.print(EVT_SCENE_NEXT);
        Serial.println("SCENE_NEXT");
      }
    }
    lastBtn3 = btn3;

    // TOGGLE_LED: update ledMasterOn on change
    if (tog != lastToggle) {
      ledMasterOn = (tog == LOW);
      Serial.print("LED master: ");
      Serial.println(ledMasterOn ? "ON" : "OFF");
    }
    lastToggle = tog;

    timestamp = millis(); PT_WAIT_UNTIL(pt, millis() - timestamp > DeBounceDelay);
  }
  PT_END(pt);
}

// =============================================================================
// Pioneer command helpers
// =============================================================================

void PulseVolumeUp()
{
  QueueCommands[QueueIndex]=VOLUMEUP;
  IncreaseQueueIndex();
  espSerial.print(EVT_VOL_UP);
  Serial.println("PULSE-UP");
}
void PulseVolumeDown()
{
  QueueCommands[QueueIndex]=VOLUMEDOWN;
  IncreaseQueueIndex();
  espSerial.print(EVT_VOL_DOWN);
  Serial.println("PULSE-DOWN");
}
void PulseTrackForward(void)
{
  QueueCommands[QueueIndex]=TRACKFF;
  IncreaseQueueIndex();
  espSerial.print(EVT_SKIP_FWD);
  Serial.println("PULSE-FF");
}
void PulseTrackBack(void)
{
  QueueCommands[QueueIndex]=TRACKPV;
  IncreaseQueueIndex();
  espSerial.print(EVT_SKIP_BACK);
  Serial.println("PULSE-PV");
}
void PulseMute(void)
{
  QueueCommands[QueueIndex]=MUTE;
  IncreaseQueueIndex();
  espSerial.print(EVT_MUTE);
  Serial.println("PULSE-MUTE");
}
void PulseTripleClick(void)
{
  QueueCommands[QueueIndex]=TRIPLECLICK;
  IncreaseQueueIndex();
  Serial.println("PULSE-TRIPLE");
}

// =============================================================================
// Encoder
// =============================================================================

int getEncoderTurn(void)
{
  int result=0;
  int newA=digitalRead(ENC_CLK);
  int newB=digitalRead(ENC_DT);
  if (newA != oldA || newB != oldB)
  {
    // something changed
    if (oldA == HIGH && newA==LOW)
      result = (oldB*2-1);
  }
  oldA=newA;
  oldB=newB;
  return result*-1;
}

// =============================================================================
// Queue management
// =============================================================================

void IncreaseQueueIndex()
{
    QueueIndex++;
    if (QueueIndex >= QUEUEMAXSIZE)
      QueueIndex=0;
}

void ClearQueue()
{
  //init the queue, (not really needed but just in case)
  static int clearloop=0;
  CurLimit=0;
  LoopOfThread=0;
  QueueIndex=0;
  QueueLastProc=0;
  for (clearloop=0;clearloop<QUEUEMAXSIZE;clearloop++)
     QueueCommands[clearloop]=0;

}
