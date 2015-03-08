simple wrapper for data processing with the GPU  

gpgpu::Process manages data with textures  
it can set, update and get data, for example:  

```cpp
gpgpu::Process proc;  
proc.init( "shader_name", int width, int height, vector<string> backbuffers_to_store_results );  
proc.set( "an_input_data_texture_on_the_shader", float* input_data );  
proc.update( int ping_pong_iterations );  
ofTexture intput_tex_data = proc.get("an_input_data_texture_on_the_shader");  
ofTexture result = proc.get();  

gpgpu::Process proc2;  
//bla  
proc2.set( "input_data_texture_xxx", result ); //set some input data with a texture  
//etc  
//etc  

//draw some results  
draw_shader.begin();  
draw_shader.setUniformTexture("result_to_draw", proc2.get(), 0);  
mesh.draw();  
draw_shader.end();  
```
