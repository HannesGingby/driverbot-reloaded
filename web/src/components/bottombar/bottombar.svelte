<script lang="ts">
  import { logout } from "$lib/auth.js";

  import Action from "../global/action.svelte";
  import VLine from "../global/v-line.svelte";
  import Account from "./account.svelte";

  import MoveLeft from "lucide-svelte/icons/move-left";
  import Signature from "lucide-svelte/icons/signature";
  import Github from "lucide-svelte/icons/github";
</script>

<div class="flex justify-between px-0.5">
  <div class="flex items-center justify-between gap-5">
    <Action
      action={logout}
      Icon={MoveLeft}
      iconHeight="17.5"
      text="Logout"
      color="text-200"
      iconOffset="2"
    />
    <div class="h-[16px]">
      <VLine />
    </div>
    <Account />
  </div>
  <div class="flex items-center justify-between gap-5">
    <Action
      action={() => console.log("Display credits")}
      Icon={Signature}
      iconHeight="17.5"
      text="Credits"
      color="text-200"
      textSize={12}
      iconOffset="2"
    />
    <Action
      action={() =>
        window.open("https://github.com/HannesGingby/driverbot-reloaded")}
      Icon={Github}
      iconHeight="17.5"
      text="Code"
      color="text-200"
      textSize={12}
      iconOffset="2"
    />
  </div>
</div>
