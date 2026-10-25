extends Area3D

signal hit(is_valid: bool)

func take_hit(is_valid: bool):
	hit.emit(is_valid)
