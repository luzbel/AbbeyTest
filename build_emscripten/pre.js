Module["preRun"].push(function () {
    addRunDependency('syncfs')

    FS.mkdir('/save')
    FS.mount(IDBFS, { autoPersist: true }, '/save')
    FS.syncfs(true, function (err) {
      if (err) throw err
      removeRunDependency('syncfs')
      console.log("FS Synced")
    })
/*
    Module['print']("Waiting for gamepad...");
    Module['addRunDependency']("gamepad");
    window.addEventListener('gamepadconnected', function()
    {
        //OK, got one
        Module['removeRunDependency']("gamepad");
    }, false);

    //chrome
    if(!!navigator.webkitGetGamepads)
    {
        var timeout = function()
        {
            if(navigator.webkitGetGamepads()[0] !== undefined)
                Module['removeRunDependency']("gamepad");
            else
                setTimeout(timeout, 100);
        }
        setTimeout(timeout, 100);
    }
    */
  });

function syncPersist(callbackPtr) {
	FS.syncfs(false, function(err) {
		console.log("syncPersist err "+err);
		if (callbackPtr)
			dynCall('vi', callbackPtr, [err ? 1 : 0]);
	});
}

// Hazla global para que esté realmente accesible por el linker
if (typeof window !== "undefined") window.syncPersist = syncPersist;
if (typeof global !== "undefined") global.syncPersist = syncPersist;
if (typeof self !== "undefined") self.syncPersist = syncPersist;
