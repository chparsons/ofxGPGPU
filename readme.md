simple wrapper for data processing with the GPU  
  
gpgpu::Process manages data with textures  
it can set, update and get, for example:  
  
gpgpu::Process proc;  
proc.init( "shader_name", int width, int height, vector<string> list_of_data_texture_names_on_the_shader );  
proc.set( "one_of_the_data_textures_from_the_list", float* data );  
proc.update( int number_of_iterations_to_ping_pong );  
ofTexture result = proc.get("one_of_the_data_textures_from_the_list");  
  
gpgpu::Process proc2;  
//bla  
proc2.set( string the_name_of_one_of_its_data_textures, result ); //set this data from a texture  
//etc  
//etc  
  
//or draw something with a result  
draw_shader.begin();  
draw_shader.setUniformTexture("result_to_draw", proc2.get("resultxxx"), procxxx.get_data_idx("resultxxx"));  
mesh.draw();  
drawShader.end();  
  
