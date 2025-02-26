extends MarginContainer

signal sgn_connect(addr: String, port: int)

func _ready() -> void:
	pass

func _on_btconnect_button_up() -> void:
	var host = $padding/grd/txtSrvAddr.text
	var sPort: String = $padding/grd/txtSrvPort.text
	if sPort.is_valid_int():
		var port = int(sPort)
		sgn_connect.emit(host, port)
