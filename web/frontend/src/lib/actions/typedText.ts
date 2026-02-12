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
  const { speed = 10, cursor = true, queue: useQueue = true } = options;

  const fullText = node.textContent || '';
  node.textContent = '';
  node.style.visibility = 'visible';

  let cursorEl: HTMLSpanElement | null = null;
  let charIndex = 0;
  let timeoutId: ReturnType<typeof setTimeout>;

  function createCursor() {
    if (!cursor) return;
    cursorEl = document.createElement('span');
    cursorEl.className = 'typed-cursor';
    cursorEl.textContent = '▌';
    node.appendChild(cursorEl);
  }

  function typeNextChar() {
    if (charIndex < fullText.length) {
      const textNode = document.createTextNode(fullText[charIndex]);
      if (cursorEl) {
        node.insertBefore(textNode, cursorEl);
      } else {
        node.appendChild(textNode);
      }
      charIndex++;
      timeoutId = setTimeout(typeNextChar, speed);
    } else {
      // Typing complete
      if (cursorEl) {
        setTimeout(() => {
          cursorEl?.classList.add('typed-cursor-done');
        }, 500);
      }
      node.dispatchEvent(new CustomEvent('typingcomplete'));

      // Signal queue that we're done
      isTyping = false;
      processQueue();
    }
  }

  function startTyping() {
    createCursor();
    typeNextChar();
  }

  // Either queue or start immediately
  if (useQueue) {
    queue.push(startTyping);
    processQueue();
  } else {
    startTyping();
  }

  return {
    destroy() {
      clearTimeout(timeoutId);
      // If destroyed while typing, let queue continue
      if (isTyping) {
        isTyping = false;
        processQueue();
      }
    }
  };
}
