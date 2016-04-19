var canvas, c;

function makepos(data, point) {
	return [point / data[0], data[1][point] / 255];
}

AddEvent('load', function() {
	canvas = document.getElementById('canvas');
	c = canvas.getContext('2d');
	c.scale(canvas.width, -canvas.height);
	c.translate(0, -1);
	c.lineWidth = 2 / Number(canvas.width);
	server = Rpc({
		send_data: function(d) {
			//console.info(d);
			c.clearRect(0, 0, 1, 1);
			c.beginPath();
			for (var y = 0; y < 255; y += 20) {
				c.moveTo(0, y / 255);
				c.lineTo(1, y / 255);
			}
			for (var x = 0; x < 768; x += 5) {
				c.moveTo(x / d[0], 0);
				c.lineTo(x / d[0], 1);
			}
			c.strokeStyle = '#ccc';
			c.stroke();
			c.beginPath();
			var pos = makepos(d, 0);
			c.moveTo(pos[0], pos[1]);
			for (var t = 1; t < d[1].length; ++t) {
				pos = makepos(d, t);
				c.lineTo(pos[0], pos[1]);
			}
			c.strokeStyle = 'black';
			c.stroke();
		}
	});
});
