/**
 * Shared Animation Utilities
 *
 * Provides consistent animation effects across the site.
 *
 * Usage:
 *   import { bodyReveal, headingType } from '$lib/animations';
 *   <p use:bodyReveal>Text content</p>
 *   <h2 use:headingType>Heading text</h2>
 */

export { textReveal, type TextRevealOptions } from '$lib/actions/textReveal';
export { typedText, type TypedTextOptions } from '$lib/actions/typedText';

// Preset configurations for common use cases

import { textReveal, type TextRevealOptions } from '$lib/actions/textReveal';
import { typedText, type TypedTextOptions } from '$lib/actions/typedText';

/**
 * Body text reveal - CRT-style line-by-line reveal with flicker
 * Use for paragraphs and body content
 */
export function bodyReveal(node: HTMLElement, options: Partial<TextRevealOptions> = {}) {
  return textReveal(node, {
    mode: 'lines',
    stagger: 100,
    duration: 1000,
    delay: 0,
    flicker: true,
    triggerOnScroll: true,
    threshold: 0.2,
    ...options
  });
}

/**
 * Heading type-in effect - typewriter animation for subheadings
 * Use for h2, h3, taglines, and emphasis text
 */
export function headingType(node: HTMLElement, options: Partial<TypedTextOptions> = {}) {
  return typedText(node, {
    speed: 40,
    cursor: true,
    queue: true,
    ...options
  });
}

/**
 * Fast type - quicker typewriter for shorter text
 * Use for labels, status messages, UI feedback
 */
export function fastType(node: HTMLElement, options: Partial<TypedTextOptions> = {}) {
  return typedText(node, {
    speed: 20,
    cursor: true,
    queue: false,
    ...options
  });
}

/**
 * Word reveal - reveals text word by word
 * Use for quotes or emphasized passages
 */
export function wordReveal(node: HTMLElement, options: Partial<TextRevealOptions> = {}) {
  return textReveal(node, {
    mode: 'words',
    stagger: 60,
    duration: 800,
    flicker: true,
    triggerOnScroll: true,
    ...options
  });
}

/**
 * Char reveal - reveals text character by character (no cursor)
 * Use for dramatic single-line reveals
 */
export function charReveal(node: HTMLElement, options: Partial<TextRevealOptions> = {}) {
  return textReveal(node, {
    mode: 'chars',
    stagger: 30,
    duration: 500,
    flicker: false,
    triggerOnScroll: true,
    ...options
  });
}
