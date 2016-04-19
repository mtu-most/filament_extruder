var _rpc_calls = new Object;
var _rpc_id = 0;

// Don't use JSON.stringify, because it doesn't properly handle NaN and Infinity.
function _rpc_tojson(obj)
{
	if (typeof obj === 'object')
	{
		if (Boolean.prototype.isPrototypeOf(obj))
			obj = Boolean(obj);
		else if (Number.prototype.isPrototypeOf(obj))
			obj = Number(obj);
		else if (String.prototype.isPrototypeOf(obj))
			obj = String(obj);
	}
	if (typeof obj === 'number')
		return String(obj);
	else if (obj === undefined || obj === null || typeof obj === 'boolean' || typeof obj === 'string')
		return JSON.stringify(obj);
	else if (typeof obj === 'function')
		return undefined;
	else if (typeof obj === 'object')
	{
		if (Array.prototype.isPrototypeOf(obj))
		{
			var r = obj.reduce(function(prev, current, index, obj)
					{
						var c = _rpc_tojson(current);
						if (c === undefined)
							c = 'null';
						prev.push(c);
						return prev;
					}, []);
			return '[' + r.join(',') + ']';
		}
		var r = [];
		for (var a in obj)
		{
			var c = _rpc_tojson(obj[a]);
			if (c === undefined)
				continue;
			r.push(JSON.stringify(String(a)) + ':' + c);
		}
		return '{' + r.join(',') + '}';
	}
	alert('unparsable object ' + String(obj) + ' passed to tojson');
	return undefined;
}

function Rpc(obj, onopen, onclose)
{
	var ret = #WEBSOCKET#;
	ret.onopen = onopen;
	ret.onclose = onclose;
	ret.onmessage = function(frame) { _rpc_message(ret, obj, frame.data); };
	ret.call = function(name, a, ka, reply)
	{
		if (a === undefined)
			a = [];
		if (ka === undefined)
			ka = {};
		var my_id;
		if (reply) {
			_rpc_id += 1;
			my_id = _rpc_id;
			_rpc_calls[my_id] = function(x) { delete _rpc_calls[my_id]; reply(x); };
		}
		else
			my_id = null;
		this.send(_rpc_tojson(['call', [my_id, name, a, ka]]));
	};
	ret.event = function(name, a, ka)
	{
		this.call(name, a, ka, null);
	};
	ret.multicall = function(args, cb, rets, from)
	{
		if (!rets)
			rets = [];
		if (!from)
			from = 0;
		if (from >= args.length) {
			if (cb)
				cb(rets);
			return;
		}
		var arg = args[from];
		this.call(arg[0], arg[1], arg[2], function(r) {
			rets.push(r);
			if (arg[3])
				arg[3] (r);
			ret.multicall(args, cb, rets, from + 1);
		});
	};
	return ret;
}

function _rpc_message(websocket, obj, frame)
{
	// Don't use JSON.parse, because it cannot handle NaN and Infinity.
	// eval seems like a security risk, but it isn't because the data
	// and this file come from the same server; if it is compromised,
	// it will just send malicious data directly.
	var data = eval('(' + frame + ')');
	var cmd = data[0];
	if (cmd == 'call')
	{
		try
		{
			var id = data[1][0];
			var ret;
			if (data[1][1] in obj)
				ret = obj[data[1][1]].apply(obj, data[1][2]);
			else if ('' in obj)
				ret = obj[''].apply(obj, [data[1][1]].concat(data[1][2]));
			else
				console.warn('Warning: undefined function ' + data[1][1] + ' called, but no default callback defined');
			if (id != null)
				websocket.send(_rpc_tojson(['return', [id, ret]]));
		}
		catch (e)
		{
			if (id != null)
				websocket.send(_rpc_tojson(['error', e]));
		}
	}
	else if (cmd == 'error')
	{
		alert('error: ' + data[1]);
	}
	else if (cmd == 'return')
	{
		_rpc_calls[data[1][0]] (data[1][1]);
	}
	else
	{
		alert('unexpected command on websocket: ' + cmd);
	}
}
