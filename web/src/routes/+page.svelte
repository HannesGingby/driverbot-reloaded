<script lang="ts">
  import { gsap } from "gsap";
  import { onMount } from "svelte";
  
  import Navbar from "../components/topbar/navbar.svelte";
  import MoveRight from "lucide-svelte/icons/move-right";
  import GitHub from "lucide-svelte/icons/github";

  import VisualGrid from "../components/global/visual-grid.svelte";

  onMount(() => {
    gsap.fromTo(
      "#gsap-top-down-opacity",
      { opacity: 0, y: -10 },
      { opacity: 1, y: 0, duration: 1, ease: "power2.inOut", stagger: 0.2 }
    );

    gsap.fromTo(
      "#gsap-bottom-up-opacity",
      { opacity: 0, y: 10 },
      { opacity: 1, y: 0, duration: 1, ease: "power2.inOut", stagger: 0.2 }
    );
  });
</script>

<svelte:head>
  <title>Driverbot</title>
</svelte:head>

<Navbar />
<main>
  <section class="mt-48 px-32 flex items-center justify-between mb-32">
    <div class="w-[50%]">
      <h1 id="gsap-top-down-opacity" class="text-xl mb-2 inline-block relative select-none">
        <span class="font-bold text-primary">Driverbot&ensp;</span><span class="text-text-200"
          >&#123;Reloaded&#125;</span
        >
        <span class="blinking-bg bg-text-100 absolute right-[-3px] select-none -z-10">&#160;&#160;</span>
      </h1>
      <p id="gsap-top-down-opacity" class="mb-12 max-w-xl">Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aenean at urna sodales, venenatis purus at, ornare dui. Sed nec lacus sit amet nunc dictum mattis. Cras condimentum tortor ac neque euismod tempor.</p>
      <a href="/dashboard" id="gsap-top-down-opacity" class="w-48 h-10 rounded-sm bg-bg-700 flex justify-center items-center gap-2 cursor-pointer hover:bg-bg-600 transition-colors">
        <p>To dashboard</p> 
        <MoveRight size="18" />
      </a>
    </div>
    <div class="flex justify-center items-center relative w-[50%] h-80">
      <div class="w-full h-full mt-16">
        <VisualGrid />
      </div>
    </div>
  </section>
  <section id="gsap-bottom-up-opacity" class="w-full px-32 flex justify-center items-center">
    <a href="https://github.com/HannesGingby/driverbot-reloaded.git" target="_blank" class="w-full h-10 rounded-lg bg-bg-800 flex justify-center items-center gap-2 cursor-pointer hover:bg-bg-600 transition-colors">
      <p>Source</p> 
      <GitHub size="18" />
    </a>
  </section>
</main>

<style>
  @keyframes blink-bg {
    0%, 100% { opacity: 1; }
    50% { opacity: 0; }
  }
  
  .blinking-bg {
    animation: blink-bg 1s infinite steps(1);
  }
</style>