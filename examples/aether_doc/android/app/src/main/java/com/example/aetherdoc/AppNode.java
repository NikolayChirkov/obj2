// Copyright 2016 Aether authors. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//   http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// =============================================================================

package com.example.aetherdoc;

import android.app.Application;
import android.util.Log;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class AppNode extends Application {

    static {
        System.loadLibrary("native-lib");
    }
    private static native String nativeInit(String path);
    private static native String nativeRelease();

    @Override
    public void onTerminate() {
        //void onActivityDestroyed(Activity var1);
        Log.d("QQQQQQ", "App::onTerminate");
        super.onTerminate();
    }

    @Override
    public void onCreate() {
        super.onCreate();
        copyAssets("state");
        String p = getFilesDir().getAbsolutePath() + File.separator + "state";
        Log.d("QQQQQQ", "nativeInit: " + nativeInit(p));
    }

    private void copyAssets(String path) {
        String dest_directory = getFilesDir().getAbsolutePath() + File.separator + path + File.separator;
        File dest_dir_file = new File(dest_directory);
        boolean b = dest_dir_file.mkdir();
        try {
            for (String asset : getAssets().list(path)) {
                String relative = path + File.separator + asset;
                try {
                    InputStream in = getAssets().open(relative);
                    try {
                        String dest = getFilesDir().getAbsolutePath() + File.separator + relative;
                        File outFile = new File(dest);
                        outFile.delete();
                        boolean bb = outFile.createNewFile();
                        OutputStream out = new FileOutputStream(outFile);
                        byte[] buffer = new byte[1024];
                        int read;
                        while ((read = in.read(buffer)) != -1) {
                            out.write(buffer, 0, read);
                        }
                        in.close();
                        out.flush();
                        out.close();
                    } catch (IOException e) {
                    }
                } catch (IOException e) {
                    copyAssets(relative);
                }
            }
        } catch (IOException e) {
        }
    }
}
