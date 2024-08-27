import { Room, Client } from "colyseus";
import { InputData, MyRoomState, Player } from "./Part4State";

export class Part4Room extends Room<MyRoomState> {
  fixedTimeStep = 1000 / 60;

  // Maximum number of players allowed in the room
  maxPlayers = 2;

  onCreate (options: any) {
    this.setState(new MyRoomState());

    // Set map dimensions
    this.state.mapWidth = 1000;
    this.state.mapHeight = 550;

    this.onMessage(0, (client, input) => {
      // Handle player input
      const player = this.state.players.get(client.sessionId);

      // Enqueue input to user input buffer
      player.inputQueue.push(input);
    });

    let elapsedTime = 0;
    this.setSimulationInterval((deltaTime) => {
      elapsedTime += deltaTime;

      while (elapsedTime >= this.fixedTimeStep) {
        elapsedTime -= this.fixedTimeStep;
        this.fixedTick(this.fixedTimeStep);
      }
    });
  }

  fixedTick(timeStep: number) {
    const velocity = 2;

    this.state.players.forEach(player => {
      let input: InputData;

      // Dequeue player inputs
      while (input = player.inputQueue.shift()) {
        if (input.left) {
          player.x -= velocity;

        } else if (input.right) {
          player.x += velocity;
        }

        if (input.up) {
          player.y -= velocity;

        } else if (input.down) {
          player.y += velocity;
        }

        player.tick = input.tick;
      }
    });
  }

  onJoin (client: Client, options: any) {
    // Check if room is full
    if (this.state.players.size >= this.maxPlayers) {
      console.log("Room is full, kicking the player");
      client.leave();
      return;
    }

    console.log(client.sessionId, "joined!");

    const player = new Player();

    // Assign specific positions based on the order of joining
    if (this.state.players.size === 0) {
      // First player position
      player.x = 350;
      player.y = 400;
    } else if (this.state.players.size === 1) {
      // Second player position
      player.x = 650;
      player.y = 400;
    }

    // Add the player to the room state
    this.state.players.set(client.sessionId, player);
  }

  onLeave (client: Client, consented: boolean) {
    console.log(client.sessionId, "left!");
    this.state.players.delete(client.sessionId);
  }

  onDispose() {
    console.log("room", this.roomId, "disposing...");
  }
}
