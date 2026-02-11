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
