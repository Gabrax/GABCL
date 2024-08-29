import { Room, Client } from "colyseus";
import { InputData, MyRoomState, Player } from "./Part4State";

enum PlayerState {
  Idle,
  Walking,
  WalkingBackward
}

export class Part4Room extends Room<MyRoomState> {
  fixedTimeStep = 1000 / 60;
  maxPlayers = 2;
  playerStates: { [sessionId: string]: { state: PlayerState, isFacingLeft: boolean } } = {};

  onCreate(options: any) {
    this.setState(new MyRoomState());
    this.state.mapWidth = 1000;
    this.state.mapHeight = 550;

    this.onMessage(0, (client, input) => {
      const player = this.state.players.get(client.sessionId);
      if (player) {
        player.inputQueue.push(input);
      }
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
    const velocity = 3;

    this.state.players.forEach((player, sessionId) => {
      let input: InputData;
      const playerState = this.playerStates[sessionId] || { state: PlayerState.Idle, isFacingLeft: false };

      while ((input = player.inputQueue.shift())) {
        if (input.left) {
          player.x -= velocity;
          if (!playerState.isFacingLeft) {
            playerState.isFacingLeft = true;
            this.broadcastPlayerFlip(sessionId, true);
          }
        } else if (input.right) {
          player.x += velocity;
          if (playerState.isFacingLeft) {
            playerState.isFacingLeft = false;
            this.broadcastPlayerFlip(sessionId, false);
          }
        }

        if (input.up) {
          player.y -= velocity;
        } else if (input.down) {
          player.y += velocity;
        }

        player.x = Math.max(80, Math.min(player.x, 1000));
        player.y = Math.max(340, Math.min(player.y, 420));
        player.tick = input.tick;

        const isMoving = input.left || input.right || input.up || input.down;
        if (isMoving) {
          if (playerState.state !== PlayerState.Walking) {
            playerState.state = PlayerState.Walking;
            this.broadcastPlayerState(sessionId, PlayerState.Walking);
          }
        } else if (!isMoving && playerState.state !== PlayerState.Idle) {
          playerState.state = PlayerState.Idle;
          this.broadcastPlayerState(sessionId, PlayerState.Idle);
        }
      }

      this.playerStates[sessionId] = playerState;
    });
  }

  onJoin(client: Client, options: any) {
    if (this.state.players.size >= this.maxPlayers) {
      console.log("Room is full, kicking the player");
      client.leave();
      return;
    }

    console.log(client.sessionId, "joined!");

    const player = new Player();
    if (this.state.players.size === 0) {
      player.x = 350;
      player.y = 400;
    } else if (this.state.players.size === 1) {
      player.x = 650;
      player.y = 400;
      player.isFacingLeft = true;
    }

    this.state.players.set(client.sessionId, player);
    this.playerStates[client.sessionId] = { state: PlayerState.Idle, isFacingLeft: false };
  }

  onLeave(client: Client, consented: boolean) {
    console.log(client.sessionId, "left!");
    this.state.players.delete(client.sessionId);
    delete this.playerStates[client.sessionId];
  }

  onDispose() {
    console.log("room", this.roomId, "disposing...");
  }

  broadcastPlayerState(sessionId: string, state: PlayerState) {
    //console.log(`Broadcasting playerStateChange: ${sessionId}, ${state}`);
    this.broadcast("playerStateChange", { sessionId, state });
}

  broadcastPlayerFlip(sessionId: string, isFacingLeft: boolean) {
      //console.log(`Broadcasting playerFlipChange: ${sessionId}, ${isFacingLeft}`);
      this.broadcast("playerFlipChange", { sessionId, isFacingLeft });
  }
}
