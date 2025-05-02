<script lang="ts">
	import { formatDateTime } from "$lib/utils.js";

	let { log } = $props();
</script>

<div
	class="**:text-sm flex flex-col bg-bg-700 rounded-lg px-5 py-4 h-full"
	>
	<div class="flex justify-between">
		<p>{log[0]}</p>
    {#if log[2]}
      <p class="text-bad">{formatDateTime(log[1])}</p>
    {:else}
      <p class="text-text-200">{formatDateTime(log[1])}</p>
    {/if}
	</div>
</div>
