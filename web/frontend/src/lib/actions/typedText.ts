import { createTimer, type Timer } from 'animejs';

export interface TypedTextOptions {
  speed?: number;       // ms per character (default: 50)
  cursor?: boolean;     // show blinking cursor (default: true)
  queue?: boolean;      // wait for previous instances (default: true)
}

// Global queue for sequential typing
const queue: Array<() => void> = [];
let isTyping = false;

function processQueue() {
  if (isTyping || queue.length === 0) return;
  isTyping = true;
  const next = queue.shift();
  next?.();
}

export function typedText(node: HTMLElement, options: TypedTextOptions = {}) {
  const { speed = 30, cursor = true, queue: useQueue = true } = options;

  const fullText = node.textContent || '';
  const chars = fullText.split('');
  node.textContent = '';
  node.style.visibility = 'visible';

  let cursorEl: HTMLSpanElement | null = null;
  let timer: Timer | null = null;
  let charIndex = 0;

  if (cursor) {
    cursorEl = document.createElement('span');
    cursorEl.className = 'typed-cursor';
    cursorEl.textContent = '▌';
    node.appendChild(cursorEl);
  }

  function startTyping() {
    timer = createTimer({
      duration: chars.length * speed,
      onUpdate: (self) => {
        const newIndex = Math.floor((self.currentTime / speed));
        while (charIndex < newIndex && charIndex < chars.length) {
          const textNode = document.createTextNode(chars[charIndex]);
          if (cursorEl) {
            node.insertBefore(textNode, cursorEl);
          } else {
            node.appendChild(textNode);
          }
          charIndex++;
        }
      },
      onComplete: () => {
        // Type any remaining chars
        while (charIndex < chars.length) {
          const textNode = document.createTextNode(chars[charIndex]);
          if (cursorEl) {
            node.insertBefore(textNode, cursorEl);
          } else {
            node.appendChild(textNode);
          }
          charIndex++;
        }

        if (cursorEl) {
          setTimeout(() => cursorEl?.classList.add('typed-cursor-done'), 500);
        }
        node.dispatchEvent(new CustomEvent('typingcomplete'));
        isTyping = false;
        processQueue();
      }
    });
  }

  if (useQueue) {
    queue.push(startTyping);
    processQueue();
  } else {
    startTyping();
  }

  return {
    destroy() {
      timer?.pause();
      if (isTyping) {
        isTyping = false;
        processQueue();
      }
    }
  };
}
