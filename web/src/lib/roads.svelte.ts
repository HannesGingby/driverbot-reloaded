export type RoadType = "forward" | "intersection" | "left_right_t" | "left_turn" | "right_turn" | "straight_left" | "straight_right";

export interface RoadTile {
  type: RoadType;
  rotation: 0 | 90 | 180 | 270;
  x: number;
  y: number;
}

export const roadTiles = $state<RoadTile[]>([
  // { type: "forward", rotation:  0, x:  0, y:  0 },  // vertical road
  // { type: "forward", rotation: 0, x:  1, y:  0 },  // horizontal
  // { type: "intersection", rotation: 0, x: 2, y: 0 },
  // { type: "forward", rotation:90, x:  2, y: 1 },
  // { type: "left_turn", rotation:0, x:  2, y: 2 },
]);
