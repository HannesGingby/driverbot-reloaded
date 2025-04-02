<script lang="ts">
  import Command from "./command.svelte";
  import Polygon from "./polygon.svelte";
  import HLine from "../global/h-line.svelte";
  import WasdButton from "./freedrive/wasd-button.svelte";
  import RangeInput from "./freedrive/range-input.svelte";

  import GitBranch from "lucide-svelte/icons/git-branch";
  import Box from "lucide-svelte/icons/box";
  import Brain from "lucide-svelte/icons/brain";
  import Move from "lucide-svelte/icons/move";
  import PanelLeftClose from "lucide-svelte/icons/panel-left-close";
  import PanelLeftOpen from "lucide-svelte/icons/panel-left-open";
  import MoveLeft from "lucide-svelte/icons/move-left";

  import { gsap } from "gsap";

  let tooltip = $state("");

  let active = $state(true);
  let freedriveActive = $state(false);

  $effect(() => {
    if (freedriveActive) {
      gsap.fromTo("#gsap-command-tab", { opacity: 0, x: -20 }, { opacity: 1, x: 0, duration: 0.4, ease: "power1.out" });
    } else if (!freedriveActive) {
      gsap.fromTo("#gsap-commands-menu", { opacity: 0, x: 20 }, { opacity: 1, x: 0, duration: 0.4, ease: "power1.out" });
    }
  })

  let wPressed = $state(false);
  let aPressed = $state(false);
  let sPressed = $state(false);
  let dPressed = $state(false);

  import { commandsSidebar, movementData } from "$lib/store.svelte.js";
  import { sendMovement } from "$lib/mqtt.js";
  import { onMount } from "svelte";

  const mapCommands = [
    {
      label: "Map unknown",
      tooltip: "Map out an unknown map.",
      action: () => console.log("do something"),
      Icon: GitBranch,
      Svg: Polygon,
      svgColor: "var(--color-primary)",
    },
    {
      label: "Map known",
      tooltip: "Map out a known map.",
      action: () => console.log("do something"),
      Icon: Box,
      Svg: Polygon,
      svgColor: "var(--color-secondary)",
    },
    {
      label: "Map manual",
      tooltip: "Map out a map manually by driving the car.",
      action: () => console.log("do something"),
      Icon: Brain,
      Svg: Polygon,
      svgColor: "var(--color-bg-600)",
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
      svgColor: "var(--color-bg-600)",
    },
  ];

  function handleKeyDown(event: KeyboardEvent) {
    if (event.key === "w") {
      movementData.driveDirection = 1;
      wPressed = true;
      sendMovement();
    } else if (event.key === "a") {
      movementData.steerDirection = -1;
      aPressed = true;
      sendMovement();
    } else if (event.key === "s") {
      movementData.driveDirection = -1;
      sPressed = true;
      sendMovement();
    } else if (event.key === "d") {
      movementData.steerDirection = 1;
      dPressed = true;
      sendMovement();
    }
  }

  function handleKeyUp(event: KeyboardEvent) {
    if (event.key === "w") {
      movementData.driveDirection = 0;
      wPressed = false;
      sendMovement();
    } else if (event.key === "a") {
      movementData.steerDirection = 0;
      aPressed = false;
      sendMovement();
    } else if (event.key === "s") {
      movementData.driveDirection = 0;
      sPressed = false;
      sendMovement();
    } else if (event.key === "d") {
      movementData.steerDirection = 0;
      dPressed = false;
      sendMovement();
    }
  }

  function handleFreedriveBack() {
    gsap.fromTo("#gsap-command-tab", { opacity: 1, x: 0 }, { opacity: 0, x: -20, duration: 0.4, ease: "power1.out" });

    setTimeout(() => {
      freedriveActive = false;
    }, 200);
  }

  onMount(() => {
    movementData.driveSpeed = 100;
    movementData.steerSpeed = 50;
  });
</script>

<svelte:window onkeydown={handleKeyDown} onkeyup={handleKeyUp} />

<aside
  class={"bg-bg-800 rounded-lg px-[28px] pt-[28px] relative z-10 transition-[width] overflow-hidden h-full w-full"}
>
  {#if active}
    {#if freedriveActive}
      <div id="gsap-command-tab">
        <div class="flex items-center mb-8">
          <button onclick={handleFreedriveBack} class="cursor-pointer">
            <MoveLeft class="pb-5" size="48" onmouseenter={() => (tooltip = "Back")} onmouseleave={() => (tooltip = "")} />
          </button>
          <h1 class="text-lg mb-5 font-bold">Movement</h1>
        </div>
        <div class="flex flex-col w-full items-center gap-2 mb-12">
          <WasdButton
            key="w"
            bind:pressed={wPressed}
            mousedown={() => {
              movementData.driveDirection = 1;
              sendMovement();
            }}
            mouseup={() => {
              movementData.driveDirection = 0;
              sendMovement();
            }}
          />
          <div class="flex justify-between gap-2">
            <WasdButton
              key="a"
              bind:pressed={aPressed}
              mousedown={() => {
                movementData.steerDirection = -1;
                sendMovement();
              }}
              mouseup={() => {
                movementData.steerDirection = 0;
                sendMovement();
              }}
            />
            <WasdButton
              key="s"
              bind:pressed={sPressed}
              mousedown={() => {
                movementData.driveDirection = -1;
                sendMovement();
              }}
              mouseup={() => {
                movementData.driveDirection = 0;
                sendMovement();
              }}
            />
            <WasdButton
              key="d"
              bind:pressed={dPressed}
              mousedown={() => {
                movementData.steerDirection = 1;
                sendMovement();
              }}
              mouseup={() => {
                movementData.steerDirection = 0;
                sendMovement();
              }}
            />
          </div>
        </div>
        <div class="mb-[52px]">
          <h1 class="text-lg mb-5 font-bold">Drive speed</h1>
          <p class="mb-1">{movementData.driveSpeed}%</p>
          <RangeInput bind:value={movementData.driveSpeed} onchange={sendMovement} name="driveSpeed" min="0" max="100" step="10" />
        </div>
        <div class="mb-[52px]">
          <h1 class="text-lg mb-5 font-bold">Steer speed</h1>
          <p class="mb-1">{movementData.steerSpeed}%</p>
          <RangeInput bind:value={movementData.steerSpeed} onchange={sendMovement} name="steerSpeed" min="0" max="100" step="10" />
        </div>
      </div>
    {:else}
      <div id="gsap-commands-menu">
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
      </div>
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
