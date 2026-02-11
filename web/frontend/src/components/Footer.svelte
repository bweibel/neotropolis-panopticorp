<script>
    import StatusSection from './StatusSection.svelte';
</script>

<footer class="notice">

  <p>This site is under construction.</p>
  <p class="small">All activities are monitored for your protection.</p>
  <StatusSection />
</footer>

<style>
  .notice {
    border: 1px solid var(--color-base);
    padding: 1.5rem 2rem;
    font-size: 0.875rem;
    margin: auto;
        text-align: center;
  }

  .notice .small {
    font-size: 0.75rem;
    opacity: 0.5;
    margin-top: 0.5rem;
  }
</style>
