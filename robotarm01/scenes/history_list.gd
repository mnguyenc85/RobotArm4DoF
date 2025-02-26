extends MarginContainer

@onready var HistoryItem = preload("res://scenes/ctrl_command.tscn")
@onready var lst: VBoxContainer = $lst

func _ready() -> void:
	pass # Replace with function body.

func add_msg(s: String, kind: int):
	if lst.get_child_count() > 5:
		var item0 = lst.get_child(0)
		lst.remove_child(item0)
		item0.queue_free()
	
	var item = HistoryItem.instantiate()
	lst.add_child(item)
	item.set_msg(s, kind)
