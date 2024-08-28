import Phaser from 'phaser';

export class Champion {
    sprite: Phaser.GameObjects.Sprite;
    scene: Phaser.Scene;
    animations: { idle: string; run: string } = { idle: '', run: '' };

    constructor(scene: Phaser.Scene, x: number, y: number, texture: string) {
        this.scene = scene;
        this.sprite = scene.add.sprite(x, y, texture);

        this.setupAnimations();
        this.sprite.play(this.animations.idle);
    }

    setupAnimations() {
        // To be implemented by subclasses
    }

    update() {
        // Update logic for the champion
    }
}
