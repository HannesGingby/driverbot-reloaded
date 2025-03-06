<script lang="ts">
  let { defaultValue, label = "", width = 46, height = 22, Icon } = $props();

  let enabled: boolean = $state(defaultValue);

  let knobSize = height - 4;
  let translateX = width - knobSize - 2;

  function toggle() {
    enabled = !enabled;
  }
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
        <Icon size={knobSize - 4} class={"stroke-text-200"} />
      </span>
    </span>
  </button>
</div>
