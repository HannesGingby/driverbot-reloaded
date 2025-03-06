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

<a class="flex flex-nowrap gap-2.5 items-center cursor-pointer" href="/">
  <div class="relative w-9 h-4.75 flex items-center justify-center">
    {#if frame === 1}
      <Frame_1 />
    {:else if frame === 2}
      <Frame_2 />
    {:else}
      <Frame_3 />
    {/if}
  </div>

  <p class="text-base">
    <span class="font-bold">Driverbot&ensp;</span><span class="text-text-200"
      >&#123;Reloaded&#125;</span
    >
  </p>
</a>
