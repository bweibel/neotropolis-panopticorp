
<div class="crt-overlay"></div>

<style>
    .crt-overlay {
        background: url(/images/rgb-bg.jpg);
        background-color: var(--color-dark);
        background-size: 16px;
        background-blend-mode: multiply;
        animation: crt-overlay-movement 1s steps(100) infinite;
        position: fixed;
        top: 0;
        bottom: 0;
        left: 0;
        right: 0;
        z-index: -1;
    }

    @keyframes crt-overlay-movement {
        0%,100% {
            opacity: 0.4;
        }

        50% {
            opacity: 0.3;
        }
    }

</style>