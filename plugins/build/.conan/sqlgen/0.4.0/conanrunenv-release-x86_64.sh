script_folder="/home/default/cc/congelado/plugins/build/.conan/sqlgen/0.4.0"
echo "echo Restoring environment" > "$script_folder/deactivate_conanrunenv-release-x86_64.sh"
for v in PostgreSQL_ROOT OPENSSL_MODULES
do
   is_defined="true"
   value=$(printenv $v) || is_defined="" || true
   if [ -n "$value" ] || [ -n "$is_defined" ]
   then
       echo export "$v='$value'" >> "$script_folder/deactivate_conanrunenv-release-x86_64.sh"
   else
       echo unset $v >> "$script_folder/deactivate_conanrunenv-release-x86_64.sh"
   fi
done

export PostgreSQL_ROOT="/home/default/.conan2/p/b/libpq53b5f79bd7446/p"
export OPENSSL_MODULES="/home/default/.conan2/p/b/opense7ece80fda80a/p/lib/ossl-modules"