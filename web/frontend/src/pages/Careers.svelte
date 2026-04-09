<script lang="ts">
  import { bodyReveal, headingType } from '$lib/animations';
  import PageTemplate from '../components/PageTemplate.svelte';
</script>

<PageTemplate>
  <div use:bodyReveal={{ delay: 1500 }}>
    <h2>Join the Panopticorp Family</h2>

    <p>Are you a motivated self-starter with a passion for total surveillance? Do you believe that privacy is just loneliness rebranded? At Panopticorp, we're always looking for bright young minds willing to sacrifice everything — including compensation — for the greater good of our shareholders.</p>

    <h3>Unpaid Internship Opportunities</h3>

    <p>Our prestigious Unpaid Internship Program offers unparalleled exposure to cutting-edge monitoring technologies. You'll gain invaluable experience in data harvesting, behavioral prediction, and citizen optimization — skills that will look great on your permanent record.</p>

    <h4>Current Openings:</h4>

    <ul>
      <li><strong>Junior Surveillance Analyst (Unpaid)</strong> — Monitor live feeds for 12-hour shifts. Coffee provided (first week only).</li>
      <li><strong>Predictive Behavior Intern (Unpaid)</strong> — Help train our AI to anticipate thoughtcrime before it happens.</li>
      <li><strong>Community Trust Ambassador (Unpaid)</strong> — Convince citizens that cameras are friends.</li>
      <li><strong>Data Entry Specialist (Unpaid)</strong> — Input citizen records into our Permanent Archive. NDA required. Memory wipe optional.</li>
    </ul>

    <h3>What We Offer</h3>

    <p>While we cannot offer monetary compensation, we provide something far more valuable: the satisfaction of knowing you've contributed to a safer, more observed society. Benefits include:</p>

    <ul>
      <li>Exposure to industry-leading surveillance methodologies</li>
      <li>A desk (shared, hot-desking rotation)</li>
      <li>Access to the employee restroom (badge required)</li>
      <li>The privilege of being monitored by the same systems you help build</li>
      <li>A letter of recommendation (pending review by Legal)</li>
    </ul>

    <h3>Apply Today</h3>

    <p>Submit your application, complete psychological profile, biometric data, and a 5,000-word essay on why you deserve to work for free. Background checks are mandatory and will be conducted retroactively to birth.</p>

    <p><em>Panopticorp is an equal opportunity observer. All applicants will be monitored regardless of race, gender, or surveillance status.</em></p>

    <h3 use:headingType>Your future is watching. Are you?</h3>
  </div>
</PageTemplate>

<style>
  h2 {
    font-family: var(--font-heading);
    font-size: clamp(1.5rem, 4vw, 2.5rem);
    margin-bottom: 1.5rem;
    color: var(--color-primary);
  }

  h3 {
    font-family: var(--font-heading);
    font-size: clamp(1.1rem, 2.5vw, 1.5rem);
    margin-top: 2rem;
    margin-bottom: 1rem;
    color: var(--color-text);
  }

  h4 {
    font-family: var(--font-tech);
    font-size: 1rem;
    margin-top: 1.5rem;
    margin-bottom: 0.75rem;
    color: var(--color-primary);
  }

  ul {
    list-style: none;
    padding-left: 0;
    margin: 1rem 0;
  }

  li {
    position: relative;
    padding-left: 1.5rem;
    margin-bottom: 0.75rem;
  }

  li::before {
    content: '>';
    position: absolute;
    left: 0;
    color: var(--color-primary);
  }

  strong {
    color: var(--color-primary);
  }

  em {
    font-style: italic;
    opacity: 0.8;
  }
</style>
