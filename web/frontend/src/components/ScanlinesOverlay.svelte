
<div class="scanlines-overlay"></div>

<style>
    .scanlines-overlay {
        background: url(/images/scanline.png);
        background-size: 10px;
        animation: scanlines-overlay-flicker .2s steps(10) infinite;
        position: fixed;
        top: 0;
        bottom: 0;
        left: 0;
        right: 0;
        z-index: 100;
        opacity: 1;
        pointer-events: none;
    }

    @keyframes scanlines-overlay-flicker {
        0%,100% {
            opacity: 0.05;
        }

        50% {
            opacity: 0.075;
        }
    }

</style>