<script lang="ts">
  import Logo from "./logo/logo.svelte";
  import Action from "../global/action.svelte";
  import Toggle from "../global/theme-toggle.svelte";

  import FileInput from "lucide-svelte/icons/file-input";
  import FileDown from "lucide-svelte/icons/file-down";
  import Moon from "lucide-svelte/icons/moon";
  import Sun from "lucide-svelte/icons/sun";

  import { theme } from "$lib/store.svelte.js";
</script>

<div class="flex justify-between flex-nowrap items-center *:w-1/3">
  <div>
    <Logo />
  </div>
  <div class="flex gap-6 flex-nowrap justify-center">
    <Action
      action={() => {
        console.log("Opening session...");
      }}
      Icon={FileInput}
      iconHeight="17.5"
      text="Open session"
      color="text-100"
      iconOffset="2"
    ></Action>
    <Action
      action={() => {
        console.log("Saving session...");
      }}
      Icon={FileDown}
      iconHeight="17.5"
      text="Save"
      color="text-100"
      iconOffset="2"
    ></Action>
  </div>
  <div class="flex justify-end">
    <Toggle defaultValue={theme.lightMode} label="Theme:" IconEnabled={Sun} IconDisabled={Moon} />
  </div>
</div>
