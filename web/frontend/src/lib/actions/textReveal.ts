import { createTimer } from 'animejs';

export interface TextRevealOptions {
  /** Animation mode: 'lines', 'words', or 'chars' */
  mode?: 'lines' | 'words' | 'chars';
  /** Delay before animation starts (ms) */
  delay?: number;
  /** Duration per element (ms) */
  duration?: number;
  /** Stagger delay between elements (ms) */
  stagger?: number;
  /** Trigger on scroll into view */
  triggerOnScroll?: boolean;
  /** Threshold for intersection observer (0-1) */
  threshold?: number;
  /** CRT flicker effect */
  flicker?: boolean;
}

const defaultOptions: Required<TextRevealOptions> = {
  mode: 'lines',
  delay: 0,
  duration: 100,
  stagger: 40,
  triggerOnScroll: true,
  threshold: 0.2,
  flicker: true
};

export function textReveal(node: HTMLElement, options: TextRevealOptions = {}) {
  const opts = { ...defaultOptions, ...options };

  // Store original content
  const originalHTML = node.innerHTML;

  // Wrap content based on mode
  wrapContent(node, opts.mode);

  // Get wrapped elements
  const elements = node.querySelectorAll('.reveal-item');

  // Set initial state
  elements.forEach((el) => {
    (el as HTMLElement).style.opacity = '0';
    (el as HTMLElement).style.transform = 'translateY(20px)';
  });

  let hasAnimated = false;
  let observer: IntersectionObserver | null = null;

  const runAnimation = () => {
    if (hasAnimated) return;
    hasAnimated = true;

    // CRT-style reveal: snap on with optional flicker
    elements.forEach((el, i) => {
      const element = el as HTMLElement;
      const baseDelay = opts.delay + i * opts.stagger;

      createTimer({
        duration: baseDelay,
        onComplete: () => {
          if (opts.flicker) {
            // Flicker effect: rapid on/off before settling
            let flickerCount = 0;
            const flickerInterval = setInterval(() => {
              element.style.opacity = flickerCount % 2 === 0 ? '1' : '0.3';
              flickerCount++;
              if (flickerCount >= 4) {
                clearInterval(flickerInterval);
                element.style.opacity = '1';
                element.style.transform = 'translateY(0) translateX(0)';
              }
            }, 30);
          } else {
            element.style.opacity = '1';
            element.style.transform = 'translateY(0)';
          }
        }
      });
    });
  };

  if (opts.triggerOnScroll) {
    observer = new IntersectionObserver(
      (entries) => {
        entries.forEach((entry) => {
          if (entry.isIntersecting) {
            runAnimation();
            observer?.disconnect();
          }
        });
      },
      { threshold: opts.threshold }
    );
    observer.observe(node);
  } else {
    setTimeout(runAnimation, opts.delay);
  }

  return {
    destroy() {
      observer?.disconnect();
      node.innerHTML = originalHTML;
    }
  };
}

function wrapContent(node: HTMLElement, mode: 'lines' | 'words' | 'chars') {
  if (mode === 'chars') {
    wrapChars(node);
  } else if (mode === 'words') {
    wrapWords(node);
  } else {
    wrapLines(node);
  }
}

function wrapLines(node: HTMLElement) {
  // For block elements, treat each child as a line
  const children = Array.from(node.children);

  if (children.length === 0) {
    // No children, wrap the text content
    const text = node.textContent || '';
    node.innerHTML = `<span class="reveal-item" style="display: inline-block;">${text}</span>`;
    return;
  }

  children.forEach((child) => {
    const wrapper = document.createElement('span');
    wrapper.className = 'reveal-item';
    wrapper.style.display = 'block';
    child.parentNode?.insertBefore(wrapper, child);
    wrapper.appendChild(child);
  });
}

function wrapWords(node: HTMLElement) {
  const walker = document.createTreeWalker(node, NodeFilter.SHOW_TEXT, null);
  const textNodes: Text[] = [];

  while (walker.nextNode()) {
    textNodes.push(walker.currentNode as Text);
  }

  textNodes.forEach((textNode) => {
    const words = textNode.textContent?.split(/(\s+)/) || [];
    const fragment = document.createDocumentFragment();

    words.forEach((word) => {
      if (word.match(/^\s+$/)) {
        fragment.appendChild(document.createTextNode(word));
      } else if (word) {
        const span = document.createElement('span');
        span.className = 'reveal-item';
        span.style.display = 'inline-block';
        span.textContent = word;
        fragment.appendChild(span);
      }
    });

    textNode.parentNode?.replaceChild(fragment, textNode);
  });
}

function wrapChars(node: HTMLElement) {
  const walker = document.createTreeWalker(node, NodeFilter.SHOW_TEXT, null);
  const textNodes: Text[] = [];

  while (walker.nextNode()) {
    textNodes.push(walker.currentNode as Text);
  }

  textNodes.forEach((textNode) => {
    const chars = textNode.textContent?.split('') || [];
    const fragment = document.createDocumentFragment();

    chars.forEach((char) => {
      if (char === ' ') {
        fragment.appendChild(document.createTextNode(' '));
      } else {
        const span = document.createElement('span');
        span.className = 'reveal-item';
        span.style.display = 'inline-block';
        span.textContent = char;
        fragment.appendChild(span);
      }
    });

    textNode.parentNode?.replaceChild(fragment, textNode);
  });
}
