<script lang="ts">
  import PanelRightClose from "lucide-svelte/icons/panel-right-close";
  import PanelRightOpen from "lucide-svelte/icons/panel-right-open";

  import Tabs from "./tabs.svelte";
  import Connection from "./tabs/connection.svelte";
  import Mission from "./tabs/mission.svelte";
  import Logs from "./tabs/logs.svelte";

  import Rss from "lucide-svelte/icons/rss";
  import ChartSpline from "lucide-svelte/icons/chart-spline";
  import ScrollText from "lucide-svelte/icons/scroll-text";

  import { infoSidebar } from "$lib/store.svelte.js";

  let tabs = [
    {
      label: "Connection",
      Icon: Rss,
      value: 1,
      component: Connection,
    },
    {
      label: "Mission",
      Icon: ChartSpline,
      value: 2,
      component: Mission,
    },
    {
      label: "Logs",
      Icon: ScrollText,
      value: 3,
      component: Logs,
    },
  ];

  let active = $state(true);
</script>

<aside
  class="bg-bg-800 rounded-lg px-[14px] pt-[14px] relative z-10 transition-[width] overflow-hidden h-full w-full"
>
  {#if active}
    <Tabs {tabs} />
  {/if}
  <button
    class="absolute bottom-[28px] left-[28px]"
    onclick={() => {
      active = !active;
      if (infoSidebar.width == 322) {
        infoSidebar.width = 72;
      } else {
        infoSidebar.width = 322;
      }
    }}
  >
    {#if active}
      <PanelRightClose size="18px" class="stroke-text-200" />
    {:else if !active}
      <PanelRightOpen size="18px" class="stroke-text-200" />
    {/if}
  </button>
</aside>
