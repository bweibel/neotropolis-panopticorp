<script>
    import { fastType } from '$lib/animations';
    import StatusSection from './StatusSection.svelte';
</script>

<footer class="notice" >
  <p use:fastType>This site is under construction.<br>----</p>
  <p use:fastType>All activities are monitored for your protection.</p>
  <p use:fastType>ちくしょう、読めたらいいのに。</p>
  <StatusSection />
</footer>

<style>
  .notice {
    font-size: 0.875rem;
    margin: auto;
  }
</style>
