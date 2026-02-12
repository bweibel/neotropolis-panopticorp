
<div class="scanlines-overlay"></div>

<style>
    .scanlines-overlay {
        background: url(/images/scanlines.jpg);
        background-size: 100%;
        animation: scanlines-overlay-flicker .1s steps(10) infinite;
        position: fixed;
        top: 0;
        bottom: 0;
        left: 0;
        right: 0;
        z-index: 100;
        opacity: 0.01;
        pointer-events: none;
    }

    @keyframes scanlines-overlay-flicker {
        0%,100% {
            opacity: 0.01;
        }

        75% {
            opacity: 0.02;
        }
    }

</style>