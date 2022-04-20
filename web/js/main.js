function onClick(e){
    let btn = e.target;
    e.preventDefault();

    if(btn.dataset.type === "mod"){
        if(btn.classList.contains("btn-on")){
            api_call("/api", {cmd:"disableMod", arg: btn.dataset.id}, function(err,resp){
                if(!err){
                    btn.classList.remove("btn-on");
                    btn.classList.add("btn-off");
                }else{
                    alert("Ошибка: "+resp);
                }
            });
        }else{
            api_call("/api", {cmd:"enableMod", arg: btn.dataset.id}, function(err,resp){
                if(!err){
                    btn.classList.remove("btn-off");
                    btn.classList.add("btn-on");
                }else{
                    alert("Ошибка: "+err);
                }
            });
        }
        return;
    }
    if(btn.dataset.type === "btn"){
        btn.classList.add("btn-pending");
        api_call("/api", {cmd:"emulateButton", arg: btn.dataset.id}, function(err,resp){
            btn.classList.remove("btn-pending");
            if(err) alert("Ошибка: "+err);
        });
    }
    if(btn.dataset.type === "speaker"){
        btn.classList.add("btn-pending");
        api_call("/api", {cmd:"beep"}, function(err,resp){
            btn.classList.remove("btn-pending");
            if(err) alert("Ошибка: "+err);
        });
    }
}

function api_call(end, props, cb){
    let xhr = new XMLHttpRequest();
    const query = Object.keys(props).map(function(x) {return encodeURIComponent(x) + '=' + encodeURIComponent(props[x]);}).join('&');
    xhr.open('GET', end + "?" + query);
    xhr.responseType = 'text';
    xhr.send();

    xhr.onload = function() {
        if(xhr.status === 200)
            cb(false, xhr.responseText);
        else
            cb(xhr.responseText);
    };
    xhr.onerror = function() {
        cb(xhr.responseText)
    };
}
document.addEventListener("DOMContentLoaded", function(event) {
    for(let el of document.getElementsByTagName("button")) el.addEventListener("click", onClick);
});