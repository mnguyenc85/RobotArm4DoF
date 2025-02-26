extends Control

@onready var lbl_status: Label = $lblStatus
@onready var txt_srvaddr: LineEdit = $cSrv/grd/txtSrvAddr
@onready var txt_srvport: LineEdit = $cSrv/grd/txtSrvPort
@onready var bt_connect: Button = $cSrv/grd/btConnect
@onready var txt_msg: LineEdit = $cMsg/vbox/txtMsg
@onready var txt_rcv: LineEdit = $cMsg/vbox/txtRcv

const Client = preload("res://scripts/client.gd")
var _client: Client = Client.new()
var _connected: bool = false
var _connecting: bool = false

func _ready() -> void:
	_client.sgn_connected.connect(_handle_client_connected)
	_client.sgn_disconnected.connect(_handle_client_disconnected)
	_client.sgn_error.connect(_handle_client_error)
	_client.sgn_data.connect(_handle_client_data)

	add_child(_client)

func _process(_delta: float) -> void:
	pass

func _handle_client_connected() -> void:
	lbl_status.text = "Client connected to server."
	bt_connect.text = "Disconnect"
	_connected = true
	_connecting = false
	
func _handle_client_disconnected() -> void:
	lbl_status.text = "Client disconnected from server."
	bt_connect.text = "Connect"
	_connected = false	

func _handle_client_error() -> void:
	pass
	
func _handle_client_data(data) -> void:
	txt_rcv.text = data.get_string_from_ascii()

func _on_btconnect_pressed() -> void:
	if _connected or _connecting:
		_client.disconnect_from_host()
		_connected = false
		_connecting = false
	else:
		var host = txt_srvaddr.text
		var port = int(txt_srvport.text)
		_client.connect_to_host(host, port)
		_connecting = true

func _on_btSend_pressed() -> void:
	var data = txt_msg.text.to_utf8_buffer()
	_client.send(data)
