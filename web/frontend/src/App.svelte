<script lang="ts">
  import Navbar from './components/Navbar.svelte';
  import Hero from './components/Hero.svelte';
  import Footer from './components/Footer.svelte';

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

<Navbar />

<main>
  <Hero />

  <section class="status-section">
    <div class="status">
      <span class="status-indicator"></span>
      <span class="status-text">{status}</span>
    </div>
  </section>

  <Footer />
</main>

<style>
  main {
    padding-top: 4rem;
    background: var(--color-bg);
    color: var(--color-text);
  }

  .status-section {
    padding: 3rem 2rem;
    text-align: center;
  }

  .status {
    display: inline-flex;
    align-items: center;
    justify-content: center;
    gap: 0.5rem;
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
</style>
