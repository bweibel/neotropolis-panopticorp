<script lang="ts">
    import { typedText } from '$lib/actions/typedText';

  import Hero from './components/Hero.svelte';
  import Footer from './components/Footer.svelte';
  import ScreenElement from './components/ScreenElement.svelte';
  import CrtOverlay from './components/CrtOverlay.svelte';
  import ScanlinesOverlay from './components/ScanlinesOverlay.svelte';
  import CrtEffectsCanvas from './components/CrtEffectsCanvas.svelte';

</script>

<main>
  <Hero />
  <ScreenElement gridColumn="1" gridRow="1"/> 
  <ScreenElement gridColumn="2" gridRow="1" /> 
  <ScreenElement gridColumn="3" gridRow="1" /> 
  <ScreenElement gridColumn="4" gridRow="1"/> 
  <ScreenElement gridColumn="5" gridRow="1" /> 
  <ScreenElement gridColumn="6" gridRow="1" /> 
  <ScreenElement gridColumn="7" gridRow="1"/> 
  <ScreenElement gridColumn="8" gridRow="1" /> 
  <ScreenElement gridColumn="9" gridRow="1" /> 
  <ScreenElement gridColumn="10" gridRow="1" /> 
  <ScreenElement gridColumn="11" gridRow="1" /> 
  <ScreenElement gridColumn="12" gridRow="1" /> 

  <ScreenElement gridColumn="1" gridRow="2" type="horizontal" width="100%"/> 
  <ScreenElement gridColumn="1" gridRow="2" /> 
  <ScreenElement gridColumn="2" gridRow="2" type="horizontal" width="100%"/> 
  <ScreenElement gridColumn="2" gridRow="2" /> 
  <ScreenElement gridColumn="3" gridRow="2" type="horizontal" width="100%"/> 
  <ScreenElement gridColumn="3" gridRow="2" /> 
  <ScreenElement gridColumn="4" gridRow="2" type="horizontal" width="100%"/> 
  <ScreenElement gridColumn="4" gridRow="2" /> 
  <ScreenElement gridColumn="5" gridRow="2" type="horizontal" width="100%"/> 
  <ScreenElement gridColumn="5" gridRow="2" /> 
  <ScreenElement gridColumn="6" gridRow="2" type="horizontal" width="100%"/> 
  <ScreenElement gridColumn="6" gridRow="2" /> 

  <ScreenElement gridColumn="7" gridRow="2" type="horizontal" width="100%"/> 
  <ScreenElement gridColumn="7" gridRow="2" /> 
  <ScreenElement gridColumn="8" gridRow="2" type="horizontal" width="100%"/> 
  <ScreenElement gridColumn="8" gridRow="2" /> 
  <ScreenElement gridColumn="9" gridRow="2" type="horizontal" width="100%"/> 
  <ScreenElement gridColumn="9" gridRow="2" /> 
  <ScreenElement gridColumn="10" gridRow="2" type="horizontal" width="100%"/> 
  <ScreenElement gridColumn="10" gridRow="2" /> 
  <ScreenElement gridColumn="11" gridRow="2" type="horizontal" width="100%"/> 
  <ScreenElement gridColumn="11" gridRow="2" /> 
  <ScreenElement gridColumn="12" gridRow="2" type="horizontal" width="100%"/> 
  <ScreenElement gridColumn="12" gridRow="2" /> 


  <div class="mission ">
    <p >At Panopticorp, we're building a new age of civic certainty. Our unsurpassed analytical capability takes the guessing out of Gaussian probability and the neuroses out of neural networks -- deus in machina, born of machine learning. If you want to make a secure society from scratch, you must first gather enough data.</p>
    <p >Panopticorp delivers integrated monitoring, predictive risk analysis, and operational coordination everywhere that people live, work, play, and move. We partner with law enforcement, but we don’t like to think of ourselves as police – more like a big brother. We’re here to protect and serve our clientele with the authority and care you need to finally take a deep breath and relax.</p>
    <p >From Portugal to Pax-Panama, the Pacific Aquicolonies to Petrozavodskgrad, the political to the personal: Panopticorp is there. Panopticorp is everywhere.</p>
    <h3 >Pervasive. Persistent. Penetrative. Panopticorp.</h3>
  </div>
  <Footer />
</main>
<ScanlinesOverlay />
<CrtOverlay />
