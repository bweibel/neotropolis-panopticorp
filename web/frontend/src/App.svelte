<script lang="ts">
  import { bodyReveal, wordReveal } from '$lib/animations';
  import PageTemplate from './components/PageTemplate.svelte';
</script>

<PageTemplate>
  <div use:bodyReveal={{ delay: 2000 }}>
    <p>At Panopticorp, we're building a new age of civic certainty. Our unsurpassed analytical capability takes the guessing out of Gaussian probability and the neuroses out of neural networks -- <em>deus in machina</em>, born of machine learning. If you want to make a secure society from scratch, you must first gather enough data.</p>
    <p>Panopticorp delivers integrated monitoring, predictive risk analysis, and operational coordination everywhere that people live, work, play, and move. We partner with law enforcement, but we don't like to think of ourselves as police – more like a big brother. We're here to protect and serve our clientele with the authority and care you need to finally take a deep breath and relax.</p>
    <p>From Portugal to Pax-Panama, the Pacific Aquicolonies to Petrozavodskgrad, the political to the personal, the public to the private: Panopticorp is there. Panopticorp is everywhere.</p>
    <h3 use:wordReveal>Pervasive. Persistent. Penetrative. Panopticorp.</h3>
  </div>
</PageTemplate>
