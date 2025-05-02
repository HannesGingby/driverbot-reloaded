<script lang="ts">
  import type { RoadTile } from "$lib/roads.svelte.js";

  let { styles, tile}: { styles?: string, tile: RoadTile } = $props();
</script>

{#if tile.type === "forward"}
  <p class={styles}>forward</p>
{:else if tile.type === "intersection"}
  <p class={styles}>intersection</p>
{:else if tile.type === "leftRightT"}
  <p class={styles}>leftRightT</p>
{:else if tile.type === "leftTurn"}
  <p class={styles}>leftTurn</p>
{:else if tile.type === "rightTurn"}
  <p class={styles}>rightTurn</p>
{:else if tile.type === "straightLeft"}
  <p class={styles}>straightLeft</p>
{:else if tile.type === "straightRight"}
  <p class={styles}>straightRight</p>
{/if}
