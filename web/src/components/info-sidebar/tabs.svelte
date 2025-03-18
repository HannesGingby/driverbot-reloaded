<script lang="ts">
  let { tabs, activeTabValue = 1 } = $props();

  const handleClick = (tabValue: number) => () => {
    activeTabValue = tabValue;
  };
</script>

<ul class="flex justify-between gap-2">
  {#each tabs as tab}
    <li>
      <button
        class={`w-[92px] h-[44px] relative rounded-lg transition-colors duration-400 flex items-center justify-center cursor-pointer overflow-hidden after:scale-90 after:opacity-0 after:transition-all
        ${
          activeTabValue === tab.value
            ? "bg-bg-600 before:absolute after:scale-none after:opacity-100 before:inset-[2px] before:rounded-[6px] before:bg-bg-600 before:z-10 after:absolute after:inset-0 after:bg-[radial-gradient(circle_at_60%_50%,_var(--color-bg-650)_0%,var(--color-bg-500)_100%)] after:from-white/20 after:to-transparent"
            : "bg-bg-700 hover:bg-bg-650"
        }`}
        onclick={handleClick(tab.value)}
      >
        <tab.Icon size="22" class="relative z-20" />
      </button>
    </li>
  {/each}
</ul>
{#each tabs as tab}
  {#if activeTabValue == tab.value}
    <div id="gsap-tab" class="pt-[48px]">
      <tab.component />
    </div>
  {/if}
{/each}