package com.example.aetherdoc;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;

import com.example.aetherdoc.databinding.ActivityMainBinding;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        // Example of a call to a native method
        TextView tv = binding.sampleText;
        copyAssets("state");
        String p = getFilesDir().getAbsolutePath() + File.separator + "state";
        tv.setText(initAppJNI(p));
    }
    public native String initAppJNI(String path);

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