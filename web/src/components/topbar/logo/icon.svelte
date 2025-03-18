<script lang="ts">
    import Frame_1 from "./frames/frame-1.svelte";
    import Frame_2 from "./frames/frame-2.svelte";
    import Frame_3 from "./frames/frame-3.svelte";
  
    let frame: number = $state(1);
    const frameCount = 3;
    const delay: number = 1000; //ms
  
    setInterval(() => {
      if (1 <= frame && frame <= frameCount) {
        frame++;
      } else {
        frame = 1;
      }
    }, delay);
</script>
  
<div class="relative w-9 h-4.75 flex items-center justify-center">
    {#if frame === 1}
    <Frame_1 color="var(--color-logo)" />
    {:else if frame === 2}
    <Frame_2 color="var(--color-logo)"/>
    {:else}
    <Frame_3 color="var(--color-logo)"/>
    {/if}
</div>
