<script lang="ts">
  import Coordinates from "./coordinates.svelte";
  import Action from "../global/action.svelte";
  import Zoom from "./zoom.svelte";

  import SquareDashed from "lucide-svelte/icons/square-dashed";
  import Hand from "lucide-svelte/icons/hand";

  import { co } from "$lib/store.svelte.js";

  let { styles = "" } = $props();
</script>

<div class={styles}>
  <div class="flex justify-between flex-nowrap items-center *:w-1/3">
    <div>
      <Coordinates x={co.mouseCo[0]} y={co.mouseCo[1]} />
    </div>
    <div class="flex flex-nowrap gap-6 items-center justify-center">
      <Action
        action={() => console.log("do something")}
        Icon={SquareDashed}
        iconHeight="17.5"
        text="Select"
        color="text-100"
        textSize={12}
      />
      <Action
        action={() => console.log("do something")}
        Icon={Hand}
        iconHeight="17.5"
        text="Pan"
        color="text-100"
        textSize={12}
        strokeWidth="1.5"
      />
    </div>
    <div class="flex justify-end">
      <Zoom />
    </div>
  </div>
</div>
