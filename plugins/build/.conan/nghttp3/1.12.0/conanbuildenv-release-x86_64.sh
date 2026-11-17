script_folder="/home/default/cc/congelado/plugins/build/.conan/nghttp3/1.12.0"
echo "echo Restoring environment" > "$script_folder/deactivate_conanbuildenv-release-x86_64.sh"
for v in 
do
   is_defined="true"
   value=$(printenv $v) || is_defined="" || true
   if [ -n "$value" ] || [ -n "$is_defined" ]
   then
       echo export "$v='$value'" >> "$script_folder/deactivate_conanbuildenv-release-x86_64.sh"
   else
       echo unset $v >> "$script_folder/deactivate_conanbuildenv-release-x86_64.sh"
   fi
done
