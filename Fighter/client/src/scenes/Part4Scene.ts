import Phaser from "phaser";
import { Room, Client } from "colyseus.js";
import { BACKEND_URL } from "../backend";
import GesturesPlugin from 'phaser3-rex-plugins/plugins/gestures-plugin.js';

enum PlayerState {
    Idle,
    Walking,
    WalkingBackward
}

export class Part4Scene extends Phaser.Scene {

    room: Room;
    currentPlayer: Phaser.Physics.Arcade.Sprite;
    playerEntities: { [sessionId: string]: Phaser.Physics.Arcade.Sprite } = {};
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

    // State machine to track player animation state
    playerState: PlayerState = PlayerState.Idle;
    isFacingLeft: boolean = false;
    lastKeyPressTime: { [key: string]: number } = { left: 0, right: 0 };
    doubleTapThreshold: number = 300; 

    constructor() {
        super({ key: "part4" });
    }

    preload(){

        this.load.image('stage1', '/stages/hq720.jpg');

        this.load.spritesheet('connect','/tryinconnect.png', {
            frameWidth: 930,   
            frameHeight: 60   
        });
        this.load.spritesheet('jin-def', '/champions/jin-full.png', {
            frameWidth: 159,   
            frameHeight: 115   
        });
        this.load.spritesheet('kazuya-def', '/champions/kazuya-full.png', {
            frameWidth: 142,   
            frameHeight: 115   
        });
    }

    async create() {

        this.cameras.main.fadeIn(1000,0,0,0);

        const background = this.add.sprite(0, 0, 'stage1').setOrigin(0, 0);
        background.setDisplaySize(1000, 550);

        this.anims.create({
            key: 'tryin',
            frames: this.anims.generateFrameNumbers('connect', { start: 0, end: 2 }),  
            frameRate: 2,  
            repeat: -1     
        });
        
        this.anims.create({
            key: 'jin-idle',
            frames: this.anims.generateFrameNumbers('jin-def', { start: 0, end: 3 }),  
            frameRate: 6,  
            repeat: -1     
        });

        this.anims.create({
            key: 'jin-walk',
            frames: this.anims.generateFrameNumbers('jin-def', { start: 4, end: 9 }),  
            frameRate: 6,  
            repeat: -1     
        });

        this.anims.create({
            key: 'jin-walkback',
            frames: this.anims.generateFrameNumbers('jin-def', { start: 9, end: 4 }),  
            frameRate: 6,  
            repeat: -1     
        });

        this.anims.create({
            key: 'kazuya-idle',
            frames: this.anims.generateFrameNumbers('kazuya-def', { start: 0, end: 5 }),  
            frameRate: 6,  
            repeat: -1     
        });

        this.anims.create({
            key: 'kazuya-walk',
            frames: this.anims.generateFrameNumbers('kazuya-def', { start: 6, end: 11 }),  
            frameRate: 6,  
            repeat: -1     
        });

        this.anims.create({
            key: 'kazuya-walkback',
            frames: this.anims.generateFrameNumbers('kazuya-def', { start: 11, end: 6 }),  
            frameRate: 6,  
            repeat: -1     
        });


        this.cursorKeys = this.input.keyboard.createCursorKeys();

        this.input.on('pointerdown', (pointer) => {
            const currentTime = this.time.now;
            const clickDelay = currentTime - this.lastKeyPressTime[pointer.id] || 0;
            this.lastKeyPressTime[pointer.id] = currentTime;

            if (clickDelay < this.doubleTapThreshold) {
                console.log("Double-click detected!");
                // Handle double-tap logic here if needed
            }
        });

        this.setupDoubleTapDetection();

        this.debugFPS = this.add.text(4, 4, "", { color: "#ff0000", });

        // connect with the room
        await this.connect();
        
        this.room.state.players.onAdd((player, sessionId) => {
            let entity: Phaser.Physics.Arcade.Sprite;

            // Determine whether the current player is Player 1 or Player 2
            if (Object.keys(this.playerEntities).length === 0) {
                // First player is Jin
                entity = this.physics.add.sprite(player.x, player.y, 'jin-def');
                entity.play('jin-idle');
                entity.setDisplaySize(350,250);
            } else if(Object.keys(this.playerEntities).length === 1){
                // Second player is Kazuya
                entity = this.physics.add.sprite(player.x, player.y, 'kazuya-def');
                entity.setDisplaySize(350,250);
                entity.setFlipX(true);
                entity.play('kazuya-idle');
            }

            this.playerEntities[sessionId] = entity;

            // is current player
            if (sessionId === this.room.sessionId) {
                this.currentPlayer = entity;

                this.localRef = this.add.rectangle(0, 0, entity.width, entity.height);
                this.localRef.setStrokeStyle(1, 0x00ff00);
                this.remoteRef = this.add.rectangle(0, 0, entity.width, entity.height);
                this.remoteRef.setStrokeStyle(1, 0xff0000);

                player.onChange(() => {
                    this.remoteRef.x = Phaser.Math.Clamp(player.x, 80, 1000);
                    this.remoteRef.y = Phaser.Math.Clamp(player.y, 340, 420);
                });
            } else {
                // listening for server updates
                player.onChange(() => {
                    entity.setData('serverX', player.x);
                    entity.setData('serverY', player.y);
                    entity.setData('state', player.state);
                    entity.setData('isFacingLeft', player.isFacingLeft);
                
                    // Update position
                    entity.x = player.x;
                    entity.y = player.y;
                
                    // Update animation state
                    this.updateAnimationState(entity, player.state, entity.texture.key);
                });
            }
        });

        this.room.state.players.onRemove((player, sessionId) => {
            const entity = this.playerEntities[sessionId];
            if (entity) {
                entity.destroy();
                delete this.playerEntities[sessionId]
            }
        });

        this.cameras.main.setBounds(0, 0, 1000, 550);
    }

    setupDoubleTapDetection() {
        this.input.keyboard.on('keydown-LEFT', (event) => {
            //console.log('Key down LEFT event:', event);
            this.handleDoubleTap('left');
        });
    
        this.input.keyboard.on('keydown-RIGHT', (event) => {
            //console.log('Key down RIGHT event:', event);
            this.handleDoubleTap('right');
        });
    }

    handleDoubleTap(direction: 'left' | 'right') {
        const currentTime = this.time.now;
        const lastPressTime = this.lastKeyPressTime[direction] || 0;

        //console.log(`Handling double-tap for ${direction}. Last press time: ${lastPressTime}, Current time: ${currentTime}`);

        if (currentTime - lastPressTime <= this.doubleTapThreshold) {
            //console.log(`Double-tap detected for ${direction}`);
            if (direction === 'left') {
                if (!this.isFacingLeft) {
                    this.isFacingLeft = true;
                    this.currentPlayer.setFlipX(this.isFacingLeft);
                    console.log("Flipping left");
                    this.room.send("playerFlipChange", { isFacingLeft: this.isFacingLeft });
                }
            } else if (direction === 'right') {
                if (this.isFacingLeft) {
                    this.isFacingLeft = false;
                    this.currentPlayer.setFlipX(this.isFacingLeft);
                    console.log("Flipping right");
                    this.room.send("playerFlipChange", { isFacingLeft: this.isFacingLeft });
                }
            }
        }

        this.lastKeyPressTime[direction] = currentTime;
    }

    async connect() {
        // Show connection status
        const connectionStatusText = this.add.sprite(55, 250, 'connect').setOrigin(0, 0);
        connectionStatusText.setDisplaySize(900, 60);
        connectionStatusText.play('tryin');
    
        const client = new Client(BACKEND_URL);
    
        try {
            this.room = await client.joinOrCreate("part4_room", {});
            connectionStatusText.destroy();
    
            // Setup message handlers after successfully joining the room
            this.setupMessageHandlers();
        } catch (e) {
            console.log("Could not connect with the server", e);
            connectionStatusText.destroy();
        }
    }
    
    setupMessageHandlers() {
        this.room.onMessage("playerStateChange", (message) => {
            //console.log("Received playerStateChange message:", message);
            const { sessionId, state } = message;
            const player = this.playerEntities[sessionId];
            if (player) {
                this.updateAnimationState(player, state, player.texture.key);
            }
        });
    
        this.room.onMessage("playerFlipChange", (message) => {
            //console.log("Received playerFlipChange message:", message);
            const { sessionId, isFacingLeft } = message;
            const player = this.playerEntities[sessionId];
            if (player) {
                player.setFlipX(isFacingLeft);
            }
        });
    }

    update(time: number, delta: number): void {
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

        const velocity = 3;
        let isMoving = false;

        // Handle input and update player's position
        if (this.cursorKeys.left.isDown) {
            this.currentPlayer.x -= velocity;
            isMoving = true;
        } else if (this.cursorKeys.right.isDown) {
            this.currentPlayer.x += velocity;
            isMoving = true;
        }

        if (this.cursorKeys.up.isDown) {
            this.currentPlayer.y -= velocity;
            isMoving = true;
        } else if (this.cursorKeys.down.isDown) {
            this.currentPlayer.y += velocity;
            isMoving = true;
        }

        // Clamp the player's position to be within the defined boundaries
        this.currentPlayer.x = Phaser.Math.Clamp(this.currentPlayer.x, 80, 1000);
        this.currentPlayer.y = Phaser.Math.Clamp(this.currentPlayer.y, 340, 420);

        // Update the local reference position for debugging
        this.localRef.x = this.currentPlayer.x;
        this.localRef.y = this.currentPlayer.y;

        // Sync the remoteRef with localRef
        this.remoteRef.x = this.localRef.x;
        this.remoteRef.y = this.localRef.y;

        // Interpolate other player entities
        for (let sessionId in this.playerEntities) {
            if (sessionId === this.room.sessionId) {
                continue;
            }

            const entity = this.playerEntities[sessionId];
            const { serverX, serverY } = entity.data.values;

            entity.x = Phaser.Math.Linear(entity.x, serverX, 0.2);
            entity.y = Phaser.Math.Linear(entity.y, serverY, 0.2);
        }

        // Enforce boundaries before sending data to the server
        this.inputPayload.left = this.cursorKeys.left.isDown;
        this.inputPayload.right = this.cursorKeys.right.isDown;
        this.inputPayload.up = this.cursorKeys.up.isDown;
        this.inputPayload.down = this.cursorKeys.down.isDown;

        // Ensure the position is within bounds before sending to the server
        const clampedX = Phaser.Math.Clamp(this.currentPlayer.x, 80, 1000);
        const clampedY = Phaser.Math.Clamp(this.currentPlayer.y, 340, 420);

        this.room.send(0, { 
            ...this.inputPayload, 
            tick: this.currentTick,
            x: clampedX,
            y: clampedY,
            //state: this.playerState,
            //isFacingLeft: this.isFacingLeft
        });
    }

    updateAnimationState(entity: Phaser.Physics.Arcade.Sprite, state: PlayerState, textureKey: string) {
        const isJin = textureKey === 'jin-def';
    
        switch (state) {
            case PlayerState.Idle:
                entity.play(isJin ? 'jin-idle' : 'kazuya-idle');
                break;
            case PlayerState.Walking:
                if (entity.flipX) {
                    entity.play(isJin ? 'jin-walkback' : 'kazuya-walkback');
                } else {
                    entity.play(isJin ? 'jin-walk' : 'kazuya-walk');
                }
                break;
            case PlayerState.WalkingBackward:
                entity.play(isJin ? 'jin-walkback' : 'kazuya-walkback');
                break;
            default:
                break;
        }
    }

    updatePlayerState(isMoving: boolean) {
        const isJin = this.currentPlayer.texture.key === 'jin-def';

        if (isMoving) {
            if ((this.isFacingLeft && this.cursorKeys.left.isDown) || 
                (!this.isFacingLeft && this.cursorKeys.right.isDown)) {
                // Walking forward
                if (this.playerState !== PlayerState.Walking ||
                    this.currentPlayer.anims.currentAnim.key !== (isJin ? 'jin-walk' : 'kazuya-walk')) {
                    this.playerState = PlayerState.Walking;
                    this.currentPlayer.play(isJin ? 'jin-walk' : 'kazuya-walk');
                }
            } else {
                // Walking backward
                if (this.playerState !== PlayerState.Walking ||
                    this.currentPlayer.anims.currentAnim.key !== (isJin ? 'jin-walkback' : 'kazuya-walkback')) {
                    this.playerState = PlayerState.Walking;
                    this.currentPlayer.play(isJin ? 'jin-walkback' : 'kazuya-walkback');
                }
            }
        } else if (!isMoving && this.playerState !== PlayerState.Idle) {
            this.playerState = PlayerState.Idle;
            this.currentPlayer.play(isJin ? 'jin-idle' : 'kazuya-idle');
        }
    }
}
