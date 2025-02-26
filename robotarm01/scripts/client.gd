extends Node

signal sgn_connected      # Connected to server
signal sgn_data           # Received data from server
signal sgn_disconnected   # Disconnected from server
signal sgn_error          # Error with connection to server

var _status: int = 0
var Status: int:
	get: return _status
var _stream: StreamPeerTCP = StreamPeerTCP.new()

func _ready() -> void:
	_status = _stream.get_status()

func _process(_delta: float) -> void:
	_stream.poll()
	var new_status: int = _stream.get_status()
	if new_status != _status:
		_status = new_status
		match _status:
			_stream.STATUS_NONE:
				print("Disconnected from host.")
				sgn_disconnected.emit()
			_stream.STATUS_CONNECTING:
				print("Connecting to host.")
			_stream.STATUS_CONNECTED:
				print("Connected to host.")
				sgn_connected.emit()
			_stream.STATUS_ERROR:
				print("Error with socket stream.")
				sgn_error.emit()

	if _status == _stream.STATUS_CONNECTED:
		var available_bytes: int = _stream.get_available_bytes()
		if available_bytes > 0:
			print("available bytes: ", available_bytes)
			var data: Array = _stream.get_partial_data(available_bytes)
			# Check for read error.
			if data[0] != OK:
				print("Error getting data from stream: ", data[0])
				sgn_error.emit()
			else:
				sgn_data.emit(data[1])

func connect_to_host(host: String, port: int) -> void:
	print("Connecting to %s:%d" % [host, port])
	# Reset status so we can tell if it changes to error again.
	_status = _stream.STATUS_NONE
	if _stream.connect_to_host(host, port) != OK:
		print("Error connecting to host.")
		sgn_error.emit()

func disconnect_from_host() -> void:
	if _status == _stream.STATUS_CONNECTED or _status == _stream.STATUS_CONNECTING:
		_stream.disconnect_from_host()

func send(data: PackedByteArray) -> bool:
	if _status != _stream.STATUS_CONNECTED:
		print("Error: Stream is not currently connected.")
		return false
	var error: int = _stream.put_data(data)
	if error != OK:
		print("Error writing to stream: ", error)
		return false
	return true
