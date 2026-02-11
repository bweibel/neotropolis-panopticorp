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

<main>
  <div class="container">
    <div class="logo">
      <h1>PANOPTICORP</h1>
      <p class="tagline">Building a Safer Tomorrow Through Total Awareness</p>
    </div>

    <div class="status">
      <span class="status-indicator"></span>
      <span class="status-text">{status}</span>
    </div>

    <div class="notice">
      <p>This site is under construction.</p>
      <p class="small">All activities are monitored for your protection.</p>
    </div>
  </div>
</main>

<style>
  main {
    min-height: 100vh;
    display: flex;
    align-items: center;
    justify-content: center;
    background: var(--color-bg);
    color: var(--color-text);
  }

  .container {
    text-align: center;
    padding: 2rem;
  }

  .logo h1 {
    font-size: 3rem;
    font-weight: 700;
    letter-spacing: 0.5em;
    margin: 0;
    color: var(--color-primary);
  }

  .tagline {
    font-size: 0.875rem;
    letter-spacing: 0.2em;
    text-transform: uppercase;
    opacity: 0.7;
    margin-top: 0.5rem;
  }

  .status {
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 0.5rem;
    margin: 3rem 0;
    font-family: var(--font-mono);
    font-size: 0.875rem;
  }

  .status-indicator {
    width: 8px;
    height: 8px;
    border-radius: 50%;
    background: var(--color-accent);
    animation: pulse 2s infinite;
  }

  @keyframes pulse {
    0%, 100% { opacity: 1; }
    50% { opacity: 0.4; }
  }

  .notice {
    border: 1px solid var(--color-border);
    padding: 1.5rem 2rem;
    font-size: 0.875rem;
  }

  .notice .small {
    font-size: 0.75rem;
    opacity: 0.5;
    margin-top: 0.5rem;
  }
</style>