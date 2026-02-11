<script lang="ts">
  function handleScroll() {
    if (window.scrollY > 0) {
      document.body.classList.add('scrolling');
    } else {
      document.body.classList.remove('scrolling');
    }
  }

  $effect(() => {
    window.addEventListener('scroll', handleScroll);
    handleScroll();

    return () => {
      window.removeEventListener('scroll', handleScroll);
    };
  });
</script>

<nav class="navbar">
  <div class="nav-logo">
    <img src="/images/logo-light.svg" alt="Panopticorp" class="logo" />
  </div>
  <ul class="nav-links">
    <!-- Navigation placeholders -->
    <li><a href="#home">Home</a></li>
    <li><a href="#about">About</a></li>
    <li><a href="#services">Services</a></li>
    <li><a href="#contact">Contact</a></li>
  </ul>
</nav>

<style>
  .navbar {
    position: fixed;
    top: 0;
    left: 0;
    right: 0;
    display: flex;
    align-items: center;
    justify-content: space-between;
    padding: 1rem 2rem;
    background: var(--color-bg);
    border-bottom: 1px solid var(--color-border);
    z-index: 1000;
    transition: background 0.3s ease, box-shadow 0.3s ease;
  }

  :global(body.scrolling) .navbar {
    background: rgba(0, 0, 0, 0.95);
    box-shadow: 0 2px 10px rgba(0, 0, 0, 0.3);
  }

  .nav-logo {
    font-size: 1.25rem;
    font-weight: 700;
    color: var(--color-primary);
  }

  .logo {
    height: 2rem;
    width: auto;
  }

  .nav-links {
    display: flex;
    gap: 2rem;
    list-style: none;
    margin: 0;
    padding: 0;
  }

  .nav-links a {
    color: var(--color-text);
    text-decoration: none;
    font-size: 0.875rem;
    letter-spacing: 0.1em;
    text-transform: uppercase;
    opacity: 0.7;
    transition: opacity 0.2s ease, color 0.2s ease;
  }

  .nav-links a:hover {
    opacity: 1;
    color: var(--color-primary);
  }
</style>
