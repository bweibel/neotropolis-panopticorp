// =============================================================================
// pioneer — Pioneer head unit control via X9C104 digital pot
// Role: Encoder input, button/toggle polling, queue-based pot commands,
//       serial event emission to ESP32 Interior Hub
// =============================================================================

// =============================================================================
// Globals — encoder
// =============================================================================

// for debugging do a find/replace on Serial with (slash)(slash)Serial

int                 EncoderValue                  = 0;
int                 PreviousEncoderVal            = 0;
int                 Delta                         = 0;
int                 PreviousPush                  = 0;
static int          oldA                          = HIGH;
static int          oldB                          = HIGH;

// time vars
unsigned long       CurrentMills                  = 0;
unsigned long       PreviousMills                 = 0;
static int          DoubleClickTime               = 350;
static int          DeBounceDelay                 = 10;
static int          MinSliceDelay                 = 1;
static int          MoreAggressiveDebounce        = 200;

// threading times
static int          WaitForUnitToComplete         = 41;         // Pioneer needs ~40ms to respond to event
static int          WaitForDisplayTime            = 850;
static int          WaitTimeForBetweenScreens     = 4100;

// hold down button for Mute method
static bool         HoldDownForMute               = true;
static int          HowLongToHoldForMute          = 500;

// ProtoThreads
static struct pt pt1, pt2, pt3;

int lastToggleState = HIGH;  // initialized in setup() from actual pin state

// =============================================================================
// Queue
// =============================================================================

#define               QUEUEMAXSIZE  200
int                   QueueCommands[QUEUEMAXSIZE];
int                   QueueIndex    = 0;
int                   QueueLastProc = 0;
static int            LoopOfThread  = 0;
static int            CurLimit      = 0;

// Command IDs — display-affecting commands must be < SCREENRANGE
#define VOLUMEUP      1
#define VOLUMEDOWN    2
#define MUTE          3
#define SCREENRANGE   4

#define TRACKFF       4
#define TRACKPV       5
#define TRIPLECLICK   6

#define NumOfStepsNeeded  1

// =============================================================================
// X9C pot
// =============================================================================

// X9C104 is 100K / 100 steps = 1K per step, set directly as percentage
// WARNING: These values were empirically tuned but the target Pioneer model
// is not documented. Possible code/hardware mismatch — verify against the
// actual installed unit before relying on this integration.
#define REST_VOLUMEUP    15
#define REST_VOLUMEDOWN  23
#define REST_TRACKFF     7
#define REST_MUTE        2
#define REST_TRACKPV     10
#define REST_TRIPLECLICK 0

X9C pot;

// =============================================================================
// Protothreads
// =============================================================================

static int protothread1(struct pt *pt)
{
  static unsigned long timestamp            = 0;
  static int           change               = 0;
  static unsigned long LastTimeDirection    = 0;
  static int           LastDirection        = 0;

  PT_BEGIN(pt);
  while(1)
  {
    change             = getEncoderTurn();
    PreviousEncoderVal = EncoderValue;
    EncoderValue       = EncoderValue + change;

    // encoder press: single-click = mute
    if (digitalRead(ENC_SW) == LOW)
    {
      if (PreviousPush == 0)
      {
        ClearQueue();
        PulseMute();
        PreviousPush = 1;
      }
      timestamp = millis(); PT_WAIT_UNTIL(pt, millis() - timestamp > DeBounceDelay);
    }
    else
    {
      PreviousPush = 0;
    }

    if (EncoderValue != PreviousEncoderVal)
    {
      Delta = Delta + EncoderValue - PreviousEncoderVal;
      if (Delta == NumOfStepsNeeded || Delta == -NumOfStepsNeeded)
      {
        if (Delta > 0)
        {
          Delta = 0;
          if (LastDirection == VOLUMEDOWN && millis() - LastTimeDirection < MoreAggressiveDebounce)
          {
            Serial.println((String)"This UP is being ignored, TimeDiff~=" + (millis() - LastTimeDirection));
          }
          else
          {
            PulseVolumeUp();
            LastDirection = VOLUMEUP;
          }
          LastTimeDirection = millis();
        }
        else
        {
          Delta = 0;
          if (LastDirection == VOLUMEUP && millis() - LastTimeDirection < MoreAggressiveDebounce)
          {
            Serial.println((String)"This DOWN is being ignored, timediff~= " + (millis() - LastTimeDirection));
          }
          else
          {
            PulseVolumeDown();
            LastDirection     = VOLUMEDOWN;
            LastTimeDirection = millis();
          }
        }
      }
    }
    timestamp = millis(); PT_WAIT_UNTIL(pt, millis() - timestamp > MinSliceDelay);
  }
  PT_END(pt);
}

static int protothread2(struct pt *pt)
{
  static unsigned long  timestamp                  = 0;
  static uint16_t       Command                    = 0;
  static unsigned long  PreviousCommandTimeStamp   = 0;
  static bool           WaitForDisplay             = false;
  static unsigned long  TimeWhenInQueue            = 0;
  static bool           InsideAQueueProcess        = false;
  static int            TempCommand                = 0;

  PT_BEGIN(pt);
  while(1)
  {
    Command  = 0;
    CurLimit = QueueIndex;
    if (CurLimit < QueueLastProc)
      CurLimit = CurLimit + QUEUEMAXSIZE;

    if (QueueLastProc != CurLimit)
    {
      Serial.println((String)"QueueIndex=" + QueueIndex + " QueueLastProc=" + QueueLastProc + " CurLimit=" + CurLimit + " LoopOfThread=" + LoopOfThread);
      for (LoopOfThread = QueueLastProc; LoopOfThread < CurLimit; LoopOfThread++)
      {
        Command = 0;
        switch (QueueCommands[LoopOfThread % QUEUEMAXSIZE])
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
          default:
            Serial.println((String)"Dont think I should hit these QueueIndex=" + QueueIndex + " QueueLastProc=" + QueueLastProc + " CurLimit=" + CurLimit);
            Command   = 0;
            timestamp = millis(); PT_WAIT_UNTIL(pt, millis() - timestamp > DeBounceDelay);
            break;
        }
        TempCommand = QueueCommands[LoopOfThread % QUEUEMAXSIZE];
        QueueCommands[LoopOfThread % QUEUEMAXSIZE] = 0;

        if (Command != 0)
        {
          timestamp = millis();
          if (timestamp - PreviousCommandTimeStamp > WaitTimeForBetweenScreens && !InsideAQueueProcess && TempCommand < SCREENRANGE)
          {
            PreviousCommandTimeStamp = timestamp;
            WaitForDisplay = true;
          }

          InsideAQueueProcess = true;
          TimeWhenInQueue     = millis();

          pot.setPot(Command, false);
          timestamp = millis(); PT_WAIT_UNTIL(pt, millis() - timestamp > WaitForUnitToComplete);
          Serial.println("CommandDone");

          pot.setPotMax(true);
          timestamp = millis(); PT_WAIT_UNTIL(pt, millis() - timestamp > WaitForUnitToComplete);

          if (WaitForDisplay)
          {
            Serial.println("Waiting for Screen");
            WaitForDisplay = false;
            timestamp = millis(); PT_WAIT_UNTIL(pt, millis() - timestamp > WaitForDisplayTime);
            Serial.println("Screen should be up, do any other commands");
          }
        }
        Command = 0;
      }
      QueueLastProc = CurLimit % QUEUEMAXSIZE;
    }
    else
    {
      timestamp = millis();
      if (timestamp - TimeWhenInQueue > WaitTimeForBetweenScreens)
      {
        if (InsideAQueueProcess)
          Serial.println("The screen is no longer on ");
        InsideAQueueProcess = false;
      }
    }
    timestamp = millis(); PT_WAIT_UNTIL(pt, millis() - timestamp > MinSliceDelay);
  }
  PT_END(pt);
}

static int protothread3(struct pt *pt)
{
  // All locals must be static — Protothreads switch/case cannot jump across initializations
  static unsigned long timestamp = 0;
  static int lastBtn1 = HIGH;
  static int lastBtn2 = HIGH;
  static int lastBtn3 = HIGH;
  static int lastBtn4 = HIGH;
  static int btn1 = HIGH;
  static int btn2 = HIGH;
  static int btn3 = HIGH;
  static int btn4 = HIGH;
  static int tog  = HIGH;

  PT_BEGIN(pt);
  while (1)
  {
    btn1 = digitalRead(BTN_1);
    btn2 = digitalRead(BTN_2);
    btn3 = digitalRead(BTN_3);
    btn4 = digitalRead(BTN_4);
    tog  = digitalRead(TOGGLE_LED);

    // BTN_1: Skip back
    if (btn1 == LOW && lastBtn1 == HIGH) {
      PulseTrackBack();
    }
    lastBtn1 = btn1;

    // BTN_2: Skip forward
    if (btn2 == LOW && lastBtn2 == HIGH) {
      PulseTrackForward();
    }
    lastBtn2 = btn2;

    // BTN_3: Scene cycle forward — suppressed when LED master off
    if (btn3 == LOW && lastBtn3 == HIGH) {
      if (ledMasterOn) {
        Serial1.print(EVT_SCENE_NEXT);
        Serial.println("SCENE_NEXT");
        currentLcdScene = (currentLcdScene + 1) % 3;
        showSceneMessage(currentLcdScene);
      }
    }
    lastBtn3 = btn3;

    // BTN_4: Scene cycle backward — suppressed when LED master off
    if (btn4 == LOW && lastBtn4 == HIGH) {
      if (ledMasterOn) {
        Serial1.print(EVT_SCENE_PREV);
        Serial.println("SCENE_PREV");
        currentLcdScene = (currentLcdScene + 2) % 3;
        showSceneMessage(currentLcdScene);
      }
    }
    lastBtn4 = btn4;

    // TOGGLE_LED: update ledMasterOn on change
    if (tog != lastToggleState) {
      ledMasterOn = (tog == LOW);
      Serial.print("LED master: ");
      Serial.println(ledMasterOn ? "ON" : "OFF");
    }
    lastToggleState = tog;

    timestamp = millis(); PT_WAIT_UNTIL(pt, millis() - timestamp > DeBounceDelay);
  }
  PT_END(pt);
}

// =============================================================================
// Pioneer command helpers
// =============================================================================

void PulseVolumeUp()
{
  QueueCommands[QueueIndex] = VOLUMEUP;
  IncreaseQueueIndex();
  Serial1.print(EVT_VOL_UP);
  Serial.println("PULSE-UP");
  showVolumeMessage(true);
  triggerKnobLedVolume();
}

void PulseVolumeDown()
{
  QueueCommands[QueueIndex] = VOLUMEDOWN;
  IncreaseQueueIndex();
  Serial1.print(EVT_VOL_DOWN);
  Serial.println("PULSE-DOWN");
  showVolumeMessage(false);
  triggerKnobLedVolume();
}

void PulseTrackForward()
{
  QueueCommands[QueueIndex] = TRACKFF;
  IncreaseQueueIndex();
  Serial1.print(EVT_SKIP_FWD);
  Serial.println("PULSE-FF");
  showTrackMessage(true);
  triggerTrackAnimation(true);
  triggerKnobLedTrack();
}

void PulseTrackBack()
{
  QueueCommands[QueueIndex] = TRACKPV;
  IncreaseQueueIndex();
  Serial1.print(EVT_SKIP_BACK);
  Serial.println("PULSE-PV");
  showTrackMessage(false);
  triggerTrackAnimation(false);
  triggerKnobLedTrack();
}

void PulseMute()
{
  QueueCommands[QueueIndex] = MUTE;
  IncreaseQueueIndex();
  Serial1.print(EVT_MUTE);
  Serial.println("PULSE-MUTE");
  showMuteMessage();
}

void PulseTripleClick()
{
  QueueCommands[QueueIndex] = TRIPLECLICK;
  IncreaseQueueIndex();
  Serial.println("PULSE-TRIPLE");
}

// =============================================================================
// Encoder
// =============================================================================

int getEncoderTurn()
{
  int result = 0;
  int newA   = digitalRead(ENC_CLK);
  int newB   = digitalRead(ENC_DT);
  if (newA != oldA || newB != oldB)
  {
    if (oldA == HIGH && newA == LOW)
      result = (oldB * 2 - 1);
  }
  oldA = newA;
  oldB = newB;
  return result;
}

// =============================================================================
// Queue management
// =============================================================================

void IncreaseQueueIndex()
{
  QueueIndex++;
  if (QueueIndex >= QUEUEMAXSIZE)
    QueueIndex = 0;
}

void ClearQueue()
{
  static int clearloop = 0;
  CurLimit      = 0;
  LoopOfThread  = 0;
  QueueIndex    = 0;
  QueueLastProc = 0;
  for (clearloop = 0; clearloop < QUEUEMAXSIZE; clearloop++)
    QueueCommands[clearloop] = 0;
}

// =============================================================================
// setup / loop
// =============================================================================

void setup()
{
  Serial1.begin(9600);

  PT_INIT(&pt1);
  PT_INIT(&pt2);
  PT_INIT(&pt3);

  pinMode(ENC_CLK,    INPUT_PULLUP);
  pinMode(ENC_DT,     INPUT_PULLUP);
  pinMode(ENC_SW,     INPUT_PULLUP);
  digitalWrite(ENC_SW, HIGH);

  pinMode(BTN_1,      INPUT_PULLUP);
  pinMode(BTN_2,      INPUT_PULLUP);
  pinMode(BTN_3,      INPUT_PULLUP);
  pinMode(BTN_4,      INPUT_PULLUP);
  pinMode(TOGGLE_LED, INPUT_PULLUP);

  ledMasterOn = true;  // TODO: gate on TOGGLE_LED when T2 toggle is wired
  lastToggleState = digitalRead(TOGGLE_LED);

  pot.begin(PIN_POT_CS, PIN_POT_INC, PIN_POT_UD);
  delay(1);
  pot.setPotMax(true);
  delay(WaitForUnitToComplete);

  ClearQueue();

  Serial.begin(115200);
  Serial.println("Started");

  lcdInit();
  ledMatrixInit();
  knobLedInit();
  otaInit();
}

void loop()
{
  protothread1(&pt1);
  protothread2(&pt2);
  protothread3(&pt3);
  updateLcd();
  updateLedMatrix();
  updateKnobLed();
  otaUpdate();
}
