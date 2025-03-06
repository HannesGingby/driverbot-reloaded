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

  let tooltip = $state("");

  let active = $state(true);
  let freedriveActive = $state(false);

  let WPressed = $state(false);
  let APressed = $state(false);
  let SPressed = $state(false);
  let DPressed = $state(false);

  import { commandsSidebar, mqttSendData } from "$lib/store.svelte.js";
  import { sendMqtt } from "$lib/mqtt.js";

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
      action: () => (freedriveActive = true),
      Icon: Move,
      Svg: Polygon,
      svgColor: "#2E2E2E",
    },
  ];

  function handleKeyDown(event: KeyboardEvent) {
    if (event.key === "w") {
      mqttSendData.direction = "forward";
      WPressed = true;
      sendMqtt();
    } else if (event.key === "a") {
      mqttSendData.steer = "left";
      APressed = true;
      sendMqtt();
    } else if (event.key === "s") {
      mqttSendData.direction = "backward";
      SPressed = true;
      sendMqtt();
    } else if (event.key === "d") {
      mqttSendData.steer = "right";
      DPressed = true;
      sendMqtt();
    }
  }

  function handleKeyUp(event: KeyboardEvent) {
    if (event.key === "w") {
      mqttSendData.direction = "";
      WPressed = false;
      sendMqtt();
    } else if (event.key === "a") {
      mqttSendData.steer = "";
      APressed = false;
      sendMqtt();
    } else if (event.key === "s") {
      mqttSendData.direction = "";
      SPressed = false;
      sendMqtt();
    } else if (event.key === "d") {
      mqttSendData.steer = "";
      DPressed = false;
      sendMqtt();
    }
  }
</script>

<svelte:window onkeydown={handleKeyDown} onkeyup={handleKeyUp} />

<aside
  class={"bg-bg-800 rounded-lg px-[28px] pt-[28px] relative z-10 transition-[width] overflow-hidden h-full w-full"}
>
  {#if active}
    {#if freedriveActive}
      <h1 class="text-lg mb-5 font-bold">Movement</h1>
      <div class="flex flex-col w-full items-center gap-2 mb-[52px]">
        <WasdButton
          key="w"
          bind:pressed={WPressed}
          mousedown={() => {
            mqttSendData.direction = "forward";
            sendMqtt();
          }}
          mouseup={() => {
            mqttSendData.direction = "";
            sendMqtt();
          }}
        />
        <div class="flex justify-between gap-2">
          <WasdButton
            key="a"
            bind:pressed={APressed}
            mousedown={() => {
              mqttSendData.steer = "left";
              sendMqtt();
            }}
            mouseup={() => {
              mqttSendData.steer = "";
              sendMqtt();
            }}
          />
          <WasdButton
            key="s"
            bind:pressed={SPressed}
            mousedown={() => {
              mqttSendData.direction = "backward";
              sendMqtt();
            }}
            mouseup={() => {
              mqttSendData.direction = "";
              sendMqtt();
            }}
          />
          <WasdButton
            key="d"
            bind:pressed={DPressed}
            mousedown={() => {
              mqttSendData.steer = "right";
              sendMqtt();
            }}
            mouseup={() => {
              mqttSendData.steer = "";
              sendMqtt();
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
