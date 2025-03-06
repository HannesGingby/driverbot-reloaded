<script lang="ts">
  import { applicationLogs } from "$lib/store.svelte.js";
  import { formatDateTime } from "$lib/utils.js";
</script>

<div class="px-[13px]">
  <section>
    <div class="mb-8">
      <h1 class="text-lg mb-[8px] font-bold">Logs</h1>
      <p class="text-sm font-normal">Application logs</p>
    </div>
    <div
      class="**:text-sm flex flex-col mb-8 bg-bg-700 rounded-lg px-5 py-4 h-128"
    >
      {#each applicationLogs.logs as log}
        <div class="flex justify-between">
          <p>{log[0]}</p>
          <p class="text-text-200">{formatDateTime(log[1])}</p>
        </div>
      {/each}
    </div>
  </section>
</div>
