<script lang="ts">
  type ScreenElementType = 'plus' | 'vertical' | 'horizontal';

  interface Props {
    type?: ScreenElementType;
    width?: string;
    height?: string;
    margin?: string;
    gridColumn?: string;
    gridRow?: string;
  }

  let {
    type = 'plus',
    width,
    height,
    marginv = '0px',
    marginh = '0px',
    gridColumn,
    gridRow
  }: Props = $props();

  const defaults: Record<ScreenElementType, { width: string; height: string; src: string }> = {
    plus: { width: '10px', height: '10px', src: '/images/screenparts/plus.svg' },
    vertical: { width: '4px', height: '10px', src: '/images/screenparts/vertical.svg' },
    horizontal: { width: '10px', height: '4px', src: '/images/screenparts/horizontal.svg' }
  };

  const config = $derived(defaults[type]);
  const finalWidth = $derived(width ?? config.width);
  const finalHeight = $derived(height ?? config.height);
</script>

<div
  class="screen-element"
  class:plus={type === 'plus'}
  class:horizontal={type === 'horizontal'}
  class:vertical={type === 'vertical'}
  style:width={finalWidth}
  style:height={finalHeight}
  style:min-width={finalWidth}
  style:min-height={finalHeight}
  style:margin-block={marginv}
  style:margin-inline={marginh}
  style:grid-column={gridColumn}
  style:grid-row={gridRow}
  style:background-image="url({config.src})"
  role="presentation"
  aria-hidden="true"
></div>

<style>
  .screen-element {
    display: block;
    pointer-events: none;
    background-repeat: repeat;

    /* will-change: filter;
    animation: crt-glow-logo-primary 10s infinite; */
    filter:
      drop-shadow(0 0 8px rgba(234, 105, 28, 0.5))
      drop-shadow(0 0 16px rgba(234, 105, 28, 0.3))
      drop-shadow(-1px 0 4px rgba(255, 60, 0, 0.25));
    }

    .plus {
        transform: translateY(-12.5px) translateX(-12.5px);
    }

    .horizontal {
        transform: translateY(-.6rem);
    }
        .vertical {
        transform: translateY(-0.6rem) ;
    }

</style>