<script lang="ts">
  import { onMount } from 'svelte';
  import { theme } from '$lib/store.svelte.js';

  let { defaultValue = false, label = "", width = 60, height = 26, IconEnabled, IconDisabled } = $props();
  let enabled: boolean = $state(defaultValue);
  let knobSize = height - 4;
  let translateX = width - knobSize - 2;
  
  function toggle() {
    enabled = !enabled;
    toggleTheme();
  }
  
  onMount(() => {
    // Check localStorage first
    const savedTheme = localStorage.getItem('theme');
    if (savedTheme) {
      theme.lightMode = savedTheme === 'light';
      // Sync enabled state with lightMode on initial load
      enabled = theme.lightMode;

    } else {
      // If no saved preference, check system preference
      theme.lightMode = window.matchMedia('(prefers-color-scheme: light)').matches;
      // Sync enabled state with lightMode on initial load
      enabled = theme.lightMode;
    }
   
    // Apply the theme
    updateTheme();
  });
  
  // Update theme when lightMode changes
  $effect(() => {
      updateTheme();
  });
  
  function updateTheme() {
    if (typeof document !== 'undefined') {
      const root = document.documentElement;
     
      if (theme.lightMode) {
        root.classList.add('light');
        localStorage.setItem('theme', 'light');
      } else {
        root.classList.remove('light');
        localStorage.setItem('theme', 'dark');
      }
    }
  }
  
  function toggleTheme() {
    theme.lightMode = !theme.lightMode;
  }
  
  // Ensure enabled state is properly initialized from defaultValue prop
  $effect(() => {
    // This effect will run when defaultValue changes
    enabled = defaultValue;
  });
</script>

<div class="flex items-center gap-2">
  {#if label}
    <p class="text-text-200 text-sm">{label}</p>
  {/if}
  <button
    type="button"
    class="relative inline-flex items-center rounded-lg bg-bg-700 transition-colors duration-400 ease-in-out cursor-pointer"
    style="height: {height}px; width: {width}px;"
    class:!bg-bg-650={enabled}
    onclick={toggle}
    aria-pressed={enabled}
  >
    <span class="sr-only">Toggle switch</span>
    <span
      class="inline-block transform rounded-lg bg-bg-900 transition duration-200 ease-in-out"
      style="width: {knobSize}px; height: {knobSize}px; transform: translateX({enabled
        ? translateX
        : 2}px); padding-left: {knobSize -
        (knobSize - 2)}px; padding-top: {knobSize - (knobSize - 1)}px;"
    >
      <span>
        {#if enabled}
          {#if IconEnabled}
            <IconEnabled size={knobSize - 4} class={"stroke-text-200"} />
          {/if}
        {:else if !enabled}
          {#if IconDisabled}
            <IconDisabled size={knobSize - 4} class={"stroke-text-200"} />
          {/if}
        {/if}
      </span>
    </span>
  </button>
</div>