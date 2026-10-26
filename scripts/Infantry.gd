extends CharacterBody3D

@export var speed: float = 5.0
@export var look_sensitivity: float = 0.002
@export var bullet_scene: PackedScene
@export var shoot_spawn: Node3D
@onready var hud = $"../HUD"

@export var hp: int = 100.0
var max_hp: int = 100
var ammo: int = 30
var heat: int = 0
var hits: int = 0
var yaw: float = 0.0
var pitch: float = 0.0
const pitch_limit_min: float = -PI/2 + 0.1
const pitch_limit_max: float = PI/2 - 0.1

func _ready():
	Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED)

func _input(event):
	if event is InputEventMouseMotion and Input.get_mouse_mode() == Input.MOUSE_MODE_CAPTURED:
		yaw -= event.relative.x * look_sensitivity
		pitch -= event.relative.y * look_sensitivity
		pitch = clamp(pitch, pitch_limit_min, pitch_limit_max)

		rotation.y = yaw
		$GimbalYaw/GimbalPitch.rotation.x = pitch


func _physics_process(delta: float) -> void:
	var input_dir: Vector2 = Input.get_vector("move_left", "move_right", "move_forward", "move_back")
	var dir: Vector3 = (transform.basis * Vector3(input_dir.x, 0, input_dir.y)).normalized()
	if dir != Vector3.ZERO:
		velocity = dir * speed
	else:
		velocity = velocity.move_toward(Vector3.ZERO, speed)

	move_and_slide()

func shoot():
	if bullet_scene == null:
		print("bullet_scene 为空")
		return
	var muzzle = get_node_or_null("GimbalYaw/GimbalPitch/Muzzle")
	if muzzle == null:
		print("找不到Muzzle枪口节点！")
		return
	var bullet = bullet_scene.instantiate()
	if bullet == null:
		print("实例化bullet失败！bullet场景内部有错误")
		return
	bullet.global_transform = muzzle.global_transform
	bullet.hit_tip.connect(hud.show_tip)
	get_parent().add_child(bullet)
	if ammo <= 0:
		return
	ammo -= 1
	print("子弹生成成功")


func take_damage(dmg: float):
	hp -= dmg
	if hp <= 0:
		hp = 0
		print("步兵阵亡")

func _process(delta):
	if Input.is_action_just_pressed("shoot"):
		shoot()


func _on_armor_front_body_entered(body: Node3D) -> void:
	if body.name == "Bullet":
		var armor = get_node("/root/Main/ArmorFront")
		armor.hp -= 10
		hits += 1
		body.queue_free()

func reset():
	ammo = 30
	heat = 0
	hits = 0
	#R键重置的时候，同步重置倒计时
	var hud = get_node("/root/Main/HUD")
	hud.time_left = 60.0
	hud.timer_running = true


func _on_tip_timer_timeout() -> void:
	pass # Replace with function body.


func _on_timer_timeout() -> void:
	pass # Replace with function body.
