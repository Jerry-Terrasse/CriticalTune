function collectData() {
    var res = Array();
    var widgets = $(".widget");
    for(widget of widgets) {
        var obj = $(widget);
        var type = obj.attr('widget-type');
        switch (type) {
            case "1":
                res.push(Number(obj.find("input").val()));
                break;
            case "2":
                res.push(obj.find("input").val() == "on" ? 1 : 0);
                break;
            case "3":
                res.push(Number(obj.find("option:selected").val()));
                break;
            case "4":
                res.push(0);
                break;
            default:
                alert('WidgetTypeError');
                break;
        }
    }
    return res;
}
function update() {
    vals = collectData();
    console.log('sending:', vals);
    $.ajax({
		type: "post",
        contentType: "text/plain",
		url: '/submit/' + Name,
		data: JSON.stringify(vals),
		dataType: "json",
		success: function(res) {
            console.log('receive:', res);
            if(res.success) {
                $('#time').html('updated ' + res['time']);
            } else {
                alert('Update Error: ' + res['errorinfo']);
            }
		}
	});
}