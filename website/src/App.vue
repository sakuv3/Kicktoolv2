<template>
    <v-app class="grey-lighten-4">
        <Toolbar />
        <v-main class="mx-4 mb-4">

            <router-view></router-view>
        </v-main>
    </v-app>
</template>

<script>
    import Toolbar from '@/components/Toolbar'
    export default {
        name: "App",
        components: {
            Toolbar
        },
        data: () => ({
            //
        })
    };
</script>