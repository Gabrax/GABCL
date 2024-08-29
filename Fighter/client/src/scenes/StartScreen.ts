import Phaser from "phaser";

export class StartScreen extends Phaser.Scene {

    constructor() {
        super({ key: "selector", active: true });
    }

    preload() {
        this.load.spritesheet('startgame', '/StartScreen/title-sheet.png', {
            frameWidth: 1920,   // Set this to the width of each frame in the spritesheet
            frameHeight: 1080,   // Set this to the height of each frame in the spritesheet
        });
    }

    create() {
        this.anims.create({
            key: 'startGameAnim',
            frames: this.anims.generateFrameNumbers('startgame', { start: 0, end: 1 }),  // Adjust the 'end' based on the number of frames
            frameRate: 2,  // Adjust the frame rate to match the original GIF's speed
            repeat: -1     // Use -1 to loop the animation
        });

        // Add the animated sprite to the scene
        const animatedSprite = this.add.sprite(0, 0, 'startgame').setOrigin(0, 0);
        animatedSprite.setDisplaySize(1000, 550);
        animatedSprite.play('startGameAnim');

        // Listen for the "Enter" key press
        this.input.keyboard.on('keydown-ENTER', () => {
            this.cameras.main.fadeOut(1000,0,0,0); 
        });

        this.cameras.main.once(Phaser.Cameras.Scene2D.Events.FADE_OUT_COMPLETE, ()=>{
            this.scene.start("part4");
        })
    }

    runScene(key) {
        this.game.scene.switch("selector", key);
    }
}
