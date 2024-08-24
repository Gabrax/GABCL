import Phaser from "phaser";

export class SceneSelector extends Phaser.Scene {

    constructor() {
        super({ key: "selector", active: true });
    }

    preload() {
        this.load.spritesheet('startgame', '/StartScreen/startbackground-spritesheet.png', {
            frameWidth: 1920,   // Set this to the width of each frame in the spritesheet
            frameHeight: 1080   // Set this to the height of each frame in the spritesheet
        });

        this.load.image('ship_0001', 'https://cdn.glitch.global/3e033dcd-d5be-4db4-99e8-086ae90969ec/ship_0001.png?v=1649945243288');
    }

    create() {
        this.anims.create({
            key: 'startGameAnim',
            frames: this.anims.generateFrameNumbers('startgame', { start: 0, end: 1 }),  // Adjust the 'end' based on the number of frames
            frameRate: 3,  // Adjust the frame rate to match the original GIF's speed
            repeat: -1     // Use -1 to loop the animation
        });

        // Add the animated sprite to the scene
        const animatedSprite = this.add.sprite(0, 0, 'startgame').setOrigin(0, 0);
        animatedSprite.setDisplaySize(1000, 550);
        animatedSprite.play('startGameAnim');

        // Listen for the "Enter" key press
        this.input.keyboard.on('keydown-ENTER', () => {
            this.runScene("part4"); // Navigate to the "Fixed Tickrate" scene
        });
    }

    runScene(key) {
        this.game.scene.switch("selector", key);
    }
}
