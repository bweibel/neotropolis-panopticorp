<script lang="ts">
  let status = $state<string>('Checking connection...');

  async function checkApi() {
    try {
      const res = await fetch('/api/health');
      const data = await res.json();
      status = data.status === 'ok' ? 'Systems Online' : 'Connection Error';
    } catch {
      status = 'Backend Offline';
    }
  }

  $effect(() => {
    checkApi();
  });
</script>

<section class="status-section">
  <div class="status">
    <span class="status-indicator"></span>
    <span class="status-text">{status}</span>
  </div>
</section>

<style>
  .status-section {
    text-align: center;
  }

  .status {
    display: inline-flex;
    align-items: center;
    justify-content: center;
    gap: 0.5rem;
    text-transform: uppercase;
    /* font-family: var(--font-mono); */
    font-size: 0.875rem;
  }

  .status-indicator {
    width: 10px;
    height: 10px;
    border-radius: 50%;
    background: var(--color-contrast);
    animation: pulse 2s ease-in-out infinite;
  }

  .status-text {
    color: var(--color-text);
    opacity: 0.8;
  }

  @keyframes pulse {
    0%, 100% {
      opacity: 1;
    }
    50% {
      opacity: 0.4;
    }
  }
</style>
