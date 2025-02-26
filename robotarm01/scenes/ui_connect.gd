extends MarginContainer

signal sgn_connect(addr: String, port: int)

func _ready() -> void:
	pass

func _on_btconnect_button_up() -> void:
	var host = $grd/txtSrvAddr.text
	var port = int($grd/txtSrvPort.text)
	sgn_connect.emit(host, port)
