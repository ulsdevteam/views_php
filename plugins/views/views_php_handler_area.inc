<?php
// $Id$

/**
 * A handler to provide an area that is constructed by the administrator using PHP.
 *
 * @ingroup views_area_handlers
 */
class views_php_handler_area extends views_handler_area {

  /**
   * Implements views_object#option_definition().
   */
  function option_definition() {
    $options = parent::option_definition();

    $options['php_output'] = array('default' => "<h4>Example PHP code</h4>\n<p>Time: <?php print date('H:i', time());?></p>\n");
    return $options;
  }

  /**
   * Implements views_handler#option_definition().
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['php_output'] = array(
      '#type' => 'textarea',
      '#title' => t('Output code'),
      '#default_value' => $this->options['php_output'],
      '#rows' => 5,
      '#description' =>
        t('Code to construct the output of this field.') . ' <strong>' . t('Use &lt;?php ?&gt; delimiters to enclose PHP code.') . '</strong><br/>' .
        t('Available variables:') . '<br/>' .
        t('$data: contains the retrieved record from the database (e.g. $data->nid).') . '<br/>' .
        t('$static: can be used to store reusable data per row.')
      ,
    );
    views_php_check_access($form['php_output']);
  }


  /**
   * Implements views_handler_area#render().
   */
  function render($empty = FALSE) {
    // Ecexute output PHP code.
    if ((!$empty || !empty($this->options['empty'])) && !empty($this->options['php_output'])) {
      $lamda_function = create_function('$view, $handler, &$static, $results', ' ?> ' . $this->options['php_output'] . ' <?php ');
      ob_start();
      $lamda_function($this->view, $this, &$this->php_static_variable, $this->view->result);
      return ob_get_clean();
    }
    return '';
  }
}
