
<div class="scanlines-overlay"></div>

<style>
    .scanlines-overlay {
        background: url(/images/scanline.png), url(/images/static.gif);

        background-size:  6px, 250px;
        background-blend-mode: darken;
        /* animation: scanlines-overlay-flicker .1s ease-in-out infinite; */
        position: fixed;
        top: 0;
        bottom: 0;
        left: 0;
        right: 0;
        z-index: 100;
        opacity: .05;
        pointer-events: none;
    }

    @keyframes scanlines-overlay-flicker {
        0%,100% {
            opacity: 0.05;
        }

        50% {
            opacity: 0.04;
        }
    }

</style>