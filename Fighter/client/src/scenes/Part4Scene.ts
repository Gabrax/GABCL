/**
 * ---------------------------
 * Phaser + Colyseus - Part 4.
 * ---------------------------
 * - Connecting with the room
 * - Sending inputs at the user's framerate
 * - Update other player's positions WITH interpolation (for other players)
 * - Client-predicted input for local (current) player
 * - Fixed tickrate on both client and server
 */

import Phaser from "phaser";
import { Room, Client } from "colyseus.js";
import { BACKEND_URL } from "../backend";

export class Part4Scene extends Phaser.Scene {
    room: Room;

    currentPlayer: Phaser.Types.Physics.Arcade.ImageWithDynamicBody;
    playerEntities: { [sessionId: string]: Phaser.Types.Physics.Arcade.ImageWithDynamicBody } = {};

    debugFPS: Phaser.GameObjects.Text;

    localRef: Phaser.GameObjects.Rectangle;
    remoteRef: Phaser.GameObjects.Rectangle;

    cursorKeys: Phaser.Types.Input.Keyboard.CursorKeys;

    inputPayload = {
        left: false,
        right: false,
        up: false,
        down: false,
        tick: undefined,
    };

    elapsedTime = 0;
    fixedTimeStep = 1000 / 60;

    currentTick: number = 0;

    constructor() {
        super({ key: "part4" });
    }

    preload(){
        this.load.image('ship_0001', 'https://cdn.glitch.global/3e033dcd-d5be-4db4-99e8-086ae90969ec/ship_0001.png?v=1649945243288');
        this.load.image('stage1', '/stages/hq720.jpg');
        this.load.spritesheet('connect','/tryinconnect.png', {
            frameWidth: 930,   
            frameHeight: 60   
        });

        this.load.spritesheet('jin-def', '/JIN/jin-full.png', {
            frameWidth: 159,   
            frameHeight: 115   
        });
    }

    async create() {
        const background = this.add.sprite(0, 0, 'stage1').setOrigin(0, 0);
        background.setDisplaySize(1000, 550);

        this.anims.create({
            key: 'jin-idle',
            frames: this.anims.generateFrameNumbers('jin-def', { start: 0, end: 3 }),  // Adjust the 'end' based on the number of frames
            frameRate: 6,  // Adjust the frame rate to match the original GIF's speed
            repeat: -1     // Use -1 to loop the animation
        });

        this.anims.create({
            key: 'tryin',
            frames: this.anims.generateFrameNumbers('connect', { start: 0, end: 2 }),  // Adjust the 'end' based on the number of frames
            frameRate: 2,  // Adjust the frame rate to match the original GIF's speed
            repeat: -1     // Use -1 to loop the animation
        });

        this.cursorKeys = this.input.keyboard.createCursorKeys();
        this.debugFPS = this.add.text(4, 4, "", { color: "#ff0000", });

        // connect with the room
        await this.connect();

        this.room.state.players.onAdd((player, sessionId) => {
            const entity = this.physics.add.sprite(player.x, player.y, 'jin-def');
            entity.play('jin-idle');
            entity.setDisplaySize(350,250);

            this.playerEntities[sessionId] = entity;

            // is current player
            if (sessionId === this.room.sessionId) {
                this.currentPlayer = entity;

                this.localRef = this.add.rectangle(0, 0, entity.width, entity.height);
                // this.localRef.setStrokeStyle(1, 0x00ff00);

                this.remoteRef = this.add.rectangle(0, 0, entity.width, entity.height);
                // this.remoteRef.setStrokeStyle(1, 0xff0000);

                player.onChange(() => {
                    this.remoteRef.x = player.x;
                    this.remoteRef.y = player.y;
                });

            } else {
                // listening for server updates
                player.onChange(() => {
                    //
                    // we're going to LERP the positions during the render loop.
                    //
                    entity.setData('serverX', player.x);
                    entity.setData('serverY', player.y);
                });

            }

        });

        // remove local reference when entity is removed from the server
        this.room.state.players.onRemove((player, sessionId) => {
            const entity = this.playerEntities[sessionId];
            if (entity) {
                entity.destroy();
                delete this.playerEntities[sessionId]
            }
        });

        // this.cameras.main.startFollow(this.ship, true, 0.2, 0.2);
        // this.cameras.main.setZoom(1);
        this.cameras.main.setBounds(0, 0, 1000, 550);
    }

    async connect() {
        // add connection status text
        const connectionStatusText = this.add.sprite(55, 250, 'connect').setOrigin(0, 0);
        connectionStatusText.setDisplaySize(900, 60);
        connectionStatusText.play('tryin');

        const client = new Client(BACKEND_URL);

        try {
            this.room = await client.joinOrCreate("part4_room", {});

            // connection successful!
            connectionStatusText.destroy();

        } catch (e) {
            // couldn't connect
            //connectionStatusText.text =  "Could not connect with the server.";
        }

    }

    update(time: number, delta: number): void {
        // skip loop if not connected yet.
        if (!this.currentPlayer) { return; }

        this.elapsedTime += delta;
        while (this.elapsedTime >= this.fixedTimeStep) {
            this.elapsedTime -= this.fixedTimeStep;
            this.fixedTick(time, this.fixedTimeStep);
        }

        this.debugFPS.text = `Frame rate: ${this.game.loop.actualFps}`;
    }

    fixedTick(time, delta) {
        this.currentTick++;
    
        const velocity = 2;
        this.inputPayload.left = this.cursorKeys.left.isDown;
        this.inputPayload.right = this.cursorKeys.right.isDown;
        this.inputPayload.up = this.cursorKeys.up.isDown;
        this.inputPayload.down = this.cursorKeys.down.isDown;
        this.inputPayload.tick = this.currentTick;
        this.room.send(0, this.inputPayload);
    
        if (this.inputPayload.left) {
            this.currentPlayer.x -= velocity;
    
        } else if (this.inputPayload.right) {
            this.currentPlayer.x += velocity;
        }
    
        if (this.inputPayload.up) {
            this.currentPlayer.y -= velocity;
    
        } else if (this.inputPayload.down) {
            this.currentPlayer.y += velocity;
        }
    
        // Clamp the player's position to be within the defined boundaries
        this.currentPlayer.x = Phaser.Math.Clamp(this.currentPlayer.x, 80, 1000);
        this.currentPlayer.y = Phaser.Math.Clamp(this.currentPlayer.y, 0, 550);
    
        // Update the local reference position for debugging
        this.localRef.x = this.currentPlayer.x;
        this.localRef.y = this.currentPlayer.y;
    
        // Interpolate other player entities
        for (let sessionId in this.playerEntities) {
            // Skip the current player
            if (sessionId === this.room.sessionId) {
                continue;
            }
    
            const entity = this.playerEntities[sessionId];
            const { serverX, serverY } = entity.data.values;
    
            // Interpolate position using linear interpolation
            entity.x = Phaser.Math.Linear(entity.x, serverX, 0.2);
            entity.y = Phaser.Math.Linear(entity.y, serverY, 0.2);
        }
    }

}
