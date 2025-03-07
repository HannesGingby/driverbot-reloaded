<script lang="ts">
  import Command from "./command.svelte";
  import Polygon from "./polygon.svelte";
  import HLine from "../global/h-line.svelte";
  import WasdButton from "./freedrive/wasd-button.svelte";

  import GitBranch from "lucide-svelte/icons/git-branch";
  import Box from "lucide-svelte/icons/box";
  import Brain from "lucide-svelte/icons/brain";
  import Move from "lucide-svelte/icons/move";
  import PanelLeftClose from "lucide-svelte/icons/panel-left-close";
  import PanelLeftOpen from "lucide-svelte/icons/panel-left-open";
  import MoveLeft from "lucide-svelte/icons/move-left";

  let tooltip = $state("");

  let active = $state(true);
  let freedriveActive = $state(false);

  let WPressed = $state(false);
  let APressed = $state(false);
  let SPressed = $state(false);
  let DPressed = $state(false);

  import { commandsSidebar, movementData } from "$lib/store.svelte.js";
  import { sendMovement } from "$lib/mqtt.js";

  const mapCommands = [
    {
      label: "Map unknown",
      tooltip: "Map out an unknown map.",
      action: () => console.log("do something"),
      Icon: GitBranch,
      Svg: Polygon,
      svgColor: "#EC5121",
    },
    {
      label: "Map known",
      tooltip: "Map out a known map.",
      action: () => console.log("do something"),
      Icon: Box,
      Svg: Polygon,
      svgColor: "#5821DA",
    },
    {
      label: "Map manual",
      tooltip: "Map out a map manually by driving the car.",
      action: () => console.log("do something"),
      Icon: Brain,
      Svg: Polygon,
      svgColor: "#2E2E2E",
    },
  ];

  const otherCommands = [
    {
      label: "Freedrive",
      tooltip: "Drive around freely with WASD.",
      action: () => {
        freedriveActive = true;
        tooltip = "";
      },
      Icon: Move,
      Svg: Polygon,
      svgColor: "#2E2E2E",
    },
  ];

  function handleKeyDown(event: KeyboardEvent) {
    if (event.key === "w") {
      movementData.direction = "forward";
      WPressed = true;
      sendMovement();
    } else if (event.key === "a") {
      movementData.steer = "left";
      APressed = true;
      sendMovement();
    } else if (event.key === "s") {
      movementData.direction = "backward";
      SPressed = true;
      sendMovement();
    } else if (event.key === "d") {
      movementData.steer = "right";
      DPressed = true;
      sendMovement();
    }
  }

  function handleKeyUp(event: KeyboardEvent) {
    if (event.key === "w") {
      movementData.direction = "";
      WPressed = false;
      sendMovement();
    } else if (event.key === "a") {
      movementData.steer = "";
      APressed = false;
      sendMovement();
    } else if (event.key === "s") {
      movementData.direction = "";
      SPressed = false;
      sendMovement();
    } else if (event.key === "d") {
      movementData.steer = "";
      DPressed = false;
      sendMovement();
    }
  }
</script>

<svelte:window onkeydown={handleKeyDown} onkeyup={handleKeyUp} />

<aside
  class={"bg-bg-800 rounded-lg px-[28px] pt-[28px] relative z-10 transition-[width] overflow-hidden h-full w-full"}
>
  {#if active}
    {#if freedriveActive}
      <div class="flex items-center mb-8">
        <button onclick={() => freedriveActive = false} class="cursor-pointer">
          <MoveLeft class="pb-5" size="48" onmouseenter={() => (tooltip = "Back")} onmouseleave={() => (tooltip = "")} />
        </button>
        <h1 class="text-lg mb-5 font-bold">Movement</h1>
      </div>
      <div class="flex flex-col w-full items-center gap-2 mb-[52px]">
        <WasdButton
          key="w"
          bind:pressed={WPressed}
          mousedown={() => {
            movementData.direction = "forward";
            sendMovement();
          }}
          mouseup={() => {
            movementData.direction = "";
            sendMovement();
          }}
        />
        <div class="flex justify-between gap-2">
          <WasdButton
            key="a"
            bind:pressed={APressed}
            mousedown={() => {
              movementData.steer = "left";
              sendMovement();
            }}
            mouseup={() => {
              movementData.steer = "";
              sendMovement();
            }}
          />
          <WasdButton
            key="s"
            bind:pressed={SPressed}
            mousedown={() => {
              movementData.direction = "backward";
              sendMovement();
            }}
            mouseup={() => {
              movementData.direction = "";
              sendMovement();
            }}
          />
          <WasdButton
            key="d"
            bind:pressed={DPressed}
            mousedown={() => {
              movementData.steer = "right";
              sendMovement();
            }}
            mouseup={() => {
              movementData.steer = "";
              sendMovement();
            }}
          />
        </div>
      </div>
    {:else}
      <h1 class="text-lg mb-5 font-bold">Explore</h1>
      <ul class="flex flex-col gap-[14px] mb-[52px]">
        {#each mapCommands as command}
          <li
            onmouseenter={() => (tooltip = command.tooltip)}
            onmouseleave={() => (tooltip = "")}
          >
            <Command
              action={command.action}
              label={command.label}
              Icon={command.Icon}
              Svg={command.Svg}
              svgColor={command.svgColor}
            />
          </li>
        {/each}
      </ul>
      <HLine styles="mb-[20px]" />
      <h1 class="text-lg mb-5 font-bold">Drive</h1>
      <ul class="flex flex-col gap-[14px] mb-[52px]">
        {#each otherCommands as command}
          <li
            onmouseenter={() => (tooltip = command.tooltip)}
            onmouseleave={() => (tooltip = "")}
          >
            <Command
              action={command.action}
              label={command.label}
              Icon={command.Icon}
              Svg={command.Svg}
              svgColor={command.svgColor}
            />
          </li>
        {/each}
      </ul>
    {/if}
    <HLine styles="mb-[20px]" />
    {#if tooltip}
      <p class="text-sm text-text-300">{tooltip}</p>
    {/if}
  {/if}
  <button
    class="absolute bottom-[28px] right-[28px]"
    onclick={() => {
      active = !active;
      if (commandsSidebar.width == 322) {
        commandsSidebar.width = 72;
      } else {
        commandsSidebar.width = 322;
      }
    }}
  >
    {#if active}
      <PanelLeftClose size="18px" class="stroke-text-200" />
    {:else if !active}
      <PanelLeftOpen size="18px" class="stroke-text-200" />
    {/if}
  </button>
</aside>
